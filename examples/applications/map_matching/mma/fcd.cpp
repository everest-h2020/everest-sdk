
#include "fcd.h"
#include <sstream>
#include <iomanip>
#include <ctime>
#include <regex>

const bool optim = false;  // generate reduced FCD stream based on embedded filter


#define TISMAXGAP 180

FCD::FCD(std::ifstream& _fcddata, GpsRectangle& _boundary) :
	fcddata(_fcddata), boundary(_boundary)
{
    //initialize();
}

void parse_datetime(std::string str, int* tis)
{
    // 2021-06-16 07:17:30.000
    std::tm tm = {};
    std::istringstream ss(str);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    //std::time_t datetime_time = std::mktime(&tm);
    *tis = tm.tm_hour*3600 + tm.tm_min*60 + tm.tm_sec;
}

void parse_pointxy(const std::string str, double* lat, double* lon)
{
    const std::string tstr("POINT(23.1 45.1)");
    std::regex rgx("POINT\\(([0-9]*[.]?[0-9]+) ([0-9]*[.]?[0-9]+)\\)");
    std::smatch match;
    std::regex_match(str.begin(), str.end(), match, rgx);
    *lon = std::stod(match[1]);
    *lat = std::stod(match[2]);
}

void FCD::initialize(int format)
{ 
    // format 2: csv - xxx,tis,lat,lon,bearing,speed,session
    // format 3: csv - xxx,datetime,POINT(lat,lon),xxx,vid,xxx,speed
    std::string line;
    std::string word;

    std::ofstream foptim("_fcd_debug.csv");

    if (optim)
    {
        foptim << "id;timestamp;geom_point;segment_id;vehicle_id;start_offset_m;speed_mps" << std::endl;
    }

    int index = 0;
    if (format == 2)
    {
        int header = 1;
        const char delim = ',';
        while (std::getline(fcddata, line))
        {
            if (header == 1) { header = 0; continue; }
            std::stringstream str(line);
            int count = 0;
            FCDrecord rec;
            while (std::getline(str, word, delim))
            {
                count++;
                if (count == 1) rec.tis = std::stoi(word);
                if (count == 2) rec.lat = std::stod(word);
                if (count == 3) rec.lon = std::stod(word);
                if (count == 5) rec.speed = std::stoi(word);
                if (count == 6) rec.session = word;
            }          
            fcdlist.push_back(std::move(rec));
        }
    }

    if (format == 3)
    {
        int header = 1;
        const char delim = ';';
        while (std::getline(fcddata, line))
        {
            if (header == 1) { header = 0; continue; }
            std::string ts, point, veh, speed;
            std::stringstream str(line);
            int count = 0;
            FCDrecord rec;
            while (std::getline(str, word, delim))
            {
                count++;
                if (count == 2) { parse_datetime(word, &rec.tis); ts = word; }
                if (count == 3) { parse_pointxy(word, &rec.lat, &rec.lon); point = word; }
                if (count == 7) { rec.speed = std::stoi(word); speed = word; }
                if (count == 5) { rec.session = word; veh = word;  }
            }

            rec.speed = (int)(rec.speed * 3.6);  // mps -> kmh

            if (optim)
            {
                bool post = false;
                std::string ss = rec.session;
                if (ss == "9884" || ss == "x2105" || ss == "x7050") post = true;
                if (post) foptim << "x1;" << ts << ";" << point << ";x4;" << veh << ";x6;" << speed << std::endl;
                if (post) fcdlist.push_back(std::move(rec));
                continue;
            }
            fcdlist.push_back(std::move(rec));

        }
    }

    reset();
}

bool FCD::InBounds(double lat, double lon)
{
    if (lat > boundary.lat1 && lat < boundary.lat2 && lon > boundary.lon1 && lon < boundary.lon2) return true;
    return false;
}

void FCD::reset()
{
    fcd_index = 0;
    trajectories.clear();
}

GpsVector FCD::get_next_vector(std::string& vid)
{
    // in/out: fcd_index 

    std::map<std::string, GpsVector>::iterator it;

    GpsVector gv_restart;
    std::string session_restart;

    while (true )
    {
        if (fcd_index == fcdlist.size())
        {
            if (!trajectories.empty())
            {
                it = trajectories.begin();
                break;
            }
            GpsVector gvnull;
            return gvnull;
        }

        auto rec = fcdlist[fcd_index];
        fcd_index++;

        if (!InBounds(rec.lat, rec.lon)) continue;
        
        const std::string session = rec.session;

        if (session == "e05d83c6f32045a393bbb9af015a7f4e")
        {
            int debug = 1;
        }

        if (trajectories.find(session) == trajectories.end() )
        {
            // does not exist -> create new
            GpsVector gv0;
            gv0.push_back(GpsPosition(rec.lat, rec.lon, rec.tis, rec.speed));
            trajectories.insert( std::make_pair(session, gv0 ));
        }
        else
        {
            // exists
            it = trajectories.find(session);

            int last_tis = it->second.back().tis;
            if ( (rec.tis - last_tis)  > TISMAXGAP || (rec.tis < last_tis) )
            {
                // break artificially

                // create new of the same session
                gv_restart.push_back(GpsPosition(rec.lat, rec.lon, rec.tis, rec.speed));
                session_restart = session;

                // conclude the previous : it->second is output
                break;
            }

            it->second.push_back(GpsPosition(rec.lat, rec.lon, rec.tis, rec.speed));
            if (it->second.size() == MAX_GV_SIZE)
            {
                //it->second is output
                break;
            }
        }

        // go over open sessions - maybe some is long concluded 
        for (it = trajectories.begin(); it != trajectories.end(); it++)
        {
            // get last item of the sassion
            const auto gps = it->second.back();

            if (rec.tis > gps.tis + TISMAXGAP)
            {
                // it->second is the output
                break;
            }
        }

    }
    
    GpsVector gv;
    vid = it->first;
    for (auto itc = it->second.begin(); itc != it->second.end(); itc++)
    {
        gv.push_back(GpsPosition(itc->lat, itc->lon, itc->tis, itc->speed));
    }
    trajectories.erase(it);

    if (!session_restart.empty())
    {
        trajectories.insert(std::make_pair(session_restart, gv_restart));
    }

	return gv;
}