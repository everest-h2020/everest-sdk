
#include "roadnetwork.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <vector>

const bool gen_optim = false; // generate optimized map graph based on mbox definition
// procedure
// 1. set NN = 1
// 2. read_map_osm (full_graph_version)
// 3. gen_optim = true
// 4. MAP_MATCH = 10
// 5. run RLS

// parsing documentation
// 
// NODES "WKT;osmid;y;x" << std::endl;
// osmid ....... reference in outputs, e.g. for sequence of nodes when routing
// y, x ........ LAT, LON
// (index) ..... created internally to ease dijkstra calculation, but convertable to osmid

// EDGES "id;geom;geom_text;u;v;key;osmid;lanes;name;highway;oneway;reversed;length;maxspeed;speed_kph" << std::endl;
//        __      _________ ___                                                     ________ _________
// id .......... is the reference id for outputs and floreon, 
// (road_id) ... internally created for internal calculation (since id could overflow), but convertable to id
// geom_text ... defines detail GPS geometry, needed for projection
// u, v ........ referene to nodes ids 
// osmid ....... ignored, because it is a possible merger of edges, thus a list
// highway ..... road classification, used for speed inference
// maxpeed, speed_kph ... used for calculating traffic index


#define PHASE_NONE 0
#define PHASE_NODES 1
#define PHASE_ROADS 2

#define M_PI 3.14159
inline double deg2rad(double deg) { return (deg * (M_PI / 180.0)); }
inline double rad2deg(double rad) { return (rad * (180 / M_PI)); }
double LLdistanceLL(double lat1, double lon1, double lat2, double lon2) {
    double theta = lon1 - lon2;
    double dist = std::sin(deg2rad(lat1)) * std::sin(deg2rad(lat2)) +
        std::cos(deg2rad(lat1)) * std::cos(deg2rad(lat2)) *
        std::cos(deg2rad(theta));
    dist = std::acos(dist);
    dist = rad2deg(dist);
    dist = dist * 60 * 1.1515;
    dist = dist * 1.609344;
    return (dist * 1000); // meters
}

std::vector<std::string> SplitIntoTokens(std::string line, char delimiter)
{
    std::vector<std::string>   result;
    std::stringstream          lineStream(line);
    std::string                cell;

    while (std::getline(lineStream, cell, delimiter))
    {
        result.push_back(cell);
    }
    // This checks for a trailing comma with no data after it.
    if (!lineStream && cell.empty())
    {
        // If there was a trailing comma then add an empty element.
        result.push_back("");
    }
    return result;
}

std::vector<std::string> getNextLineAndSplitIntoTokens(std::istream& str)
{
    std::string line;
    std::getline(str, line);
    return SplitIntoTokens(line, ';');
}

std::vector<double> parse_linestring(std::string linestring)
{
    std::vector<double> z;
    std::string toErase = "LINESTRING(";
    size_t pos = linestring.find(toErase);
    if (pos == std::string::npos)
    {
        toErase = "LINESTRING (";
        pos = linestring.find(toErase);
    }
    linestring.erase(pos, toErase.length());
    toErase = ")";
    pos = linestring.find(toErase);
    linestring.erase(pos, toErase.length());
    auto tokens = SplitIntoTokens(linestring, ',');
    for (int i = 0; i < tokens.size(); i++)
    {
        std::string gps = tokens[i];
        gps.erase(0, gps.find_first_not_of(' '));
        auto vals = SplitIntoTokens(gps, ' ');
        double lon = std::atof(vals[0].c_str());
        double lat = std::atof(vals[1].c_str());
        z.push_back(lat);
        z.push_back(lon);
    }
    return z;
}

std::string remove_quotes(std::string s)
{
    s.erase(remove(s.begin(), s.end(), '\"'), s.end());
    return s;
}

std::string remove_ws(std::string s)
{
    s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());
    return s;
}

int estim_speed(int speed1, int speed2, std::string category)
{
    if (speed1 > 0) return speed1;
    if (speed2 > 0) return speed2;
    if (category == "motorway" || category == "motorway_link") return 130;
    if (category == "trunk" || category == "trunk_link") return 60;
    if (category == "primary" || category == "primary_link") return 50;
    if (category == "secondary" || category == "secondary_link") return 50;
    if (category == "track") return 40;
    if (category == "tertiary") return 30;
    if (category == "residential") return 30;
    if (category == "service") return 30;
    if (category == "unclassified") return 30;
    std::cerr << "missing speed for " << category << std::endl;
    return 30;
}

bool allowed_category(std::string category)
{
    if (category == "tertiary") return false;
    if (category == "tertiary_link") return false;
    if (category == "residential") return false;
    if (category == "service") return false;
    if (category == "unclassified") return false;
    if (category == "track") return false;
    return true;
}


void read_map_osm(int mf, MapCell &jmap, const std::string &vfilename, const std::string& efilename, const GpsRectangle& rect) {
  std::ifstream vfile(vfilename); // vertex
  std::ifstream efile(efilename); // edges

  std::ofstream voptim("_nodes_optim.csv");
  std::ofstream eoptim("_edges_optim.csv");
  if (gen_optim)
  {
      voptim << "WKT;osmid;y;x" << std::endl;
      eoptim << "id;geom;geom_text;u;v;key;osmid;lanes;name;highway;oneway;reversed;length;maxspeed;speed_kph" << std::endl;
  }

  double mlat = 0.0045;   // 500m Prague
  double mlon = 0.0070;   // 500m Prague

  int vid = 0;
  int eid = 0;
  int gid = 0;

  int nspeeds = 0;

  bool readflow = false;
  while (!vfile.eof()) {
      auto line = getNextLineAndSplitIntoTokens(vfile);
      if (!readflow)
      {
          if (mf == 2 && line[2] == "osmid" && line[3] == "y" && remove_ws(line[4]) == "x")
          {
              readflow = true;
              continue;
          }
          if (mf == 3 && line[1] == "osmid" && remove_ws(line[2]) == "y" && remove_ws(line[3]) == "x")
          {
              readflow = true;
              continue;
          }
          std::cerr << "format incompatibility of " << vfilename << std::endl;
          exit(1);
      }

      if (line.size() < 4) break;

      id_g node_id = -1;
      double node_lat = 0;
      double node_lon = 0;
      if (mf == 2) // v1
      {
          node_id = std::atoll(line[2].c_str());
          node_lat = std::atof(line[3].c_str());
          node_lon = std::atof(line[4].c_str());
      }
      if (mf == 3) // v2
      {
          node_id = std::atoll(remove_quotes(line[1]).c_str());
          node_lat = std::atof(line[2].c_str());
          node_lon = std::atof(line[3].c_str());
      }

      // filtering through rect
      if (node_lat > rect.lat1-mlat && node_lat < rect.lat2+mlat && node_lon > rect.lon1-mlon && node_lon < rect.lon2+mlon)
      {
          jmap.add(Vertex(vid++, node_id, node_lat, node_lon));
          if (gen_optim) voptim << "x;" << node_id << ";" << node_lat << ";" << node_lon << std::endl;
      }

    }

  readflow = false;
  int count = 0;
  while (!efile.eof()) {
      count++;
      //std::cout << "edge count:" << count << std::endl;
      auto line = getNextLineAndSplitIntoTokens(efile);
      if (!readflow)
      {
          if (mf == 2 && line[2] == "u" && line[3] == "v" && remove_ws(line[5]) == "osmid")
          {
              readflow = true;
              continue;
          }
          if (mf == 3 && line[0] == "id" && line[3] == "u" && line[4] == "v" && line[13] == "maxspeed" && remove_ws(line[14]) == "speed_kph")
          {
              readflow = true;
              continue;
          }
          std::cerr << "format incompatibility of " << efilename << std::endl;
          exit(1);
      }
      if (line.size() <= 14) break;

      id_g node_from = -1;
      id_g node_to = -1;
      id_g road_id = -1;
      int maxspeed1 = -1;
      int maxspeed2 = -1;
      std::string category = "";
      int maxspeed = -1;
      std::string linestring = "";
      if (mf == 2)
      {
          node_from = std::atoll(line[2].c_str());
          node_to = std::atoll(line[3].c_str());
          road_id = std::atoll(line[0].c_str());
          maxspeed = std::atoi(line[12].c_str());
          linestring = line[1].c_str();
      }
      if (mf == 3)
      {
          node_from = std::atoll(line[3].c_str());
          node_to = std::atoll(line[4].c_str());
          road_id = std::atoll(line[0].c_str());
          maxspeed1 = std::atoi(line[13].c_str());
          maxspeed2 = std::atoi(line[14].c_str());
          category = line[9].c_str();
          maxspeed = estim_speed(maxspeed1, maxspeed2, category);          
          linestring = line[2].c_str();
      }

      // filter category
      if (!allowed_category(category))
      {
          continue;
      }

      // count problematic speeds
      if (maxspeed <= 0) nspeeds++;

      // check
      /*
      int hits = 0;
      for (id_t i = 0; i < jmap.vertices.size(); i++) {
          if (jmap.vertices.at(i).vertex_id == node_from) hits++;
          if (jmap.vertices.at(i).vertex_id == node_to) hits++;
          if (hits == 2) break;
      }
      if (hits != 2)
      {
          std::cerr << "node reference problem " << node_from << " or " << node_to << std::endl;
          continue;
      }
      */

      // geometry attaching to edge
      int gid_start = gid;
      auto vals = parse_linestring(linestring);
      for (int j = 2; j < vals.size(); j = j + 2)
      {
          double lat1 = vals[j-2];
          double lon1 = vals[j-1];
          double lat2 = vals[j];
          double lon2 = vals[j+1];
          jmap.add(Geom(gid++, -1, lat1, lon1, lat2, lon2));
      }
      int gid_end = gid - 1;

      // edge add
      bool mustfind = false;
      const auto v_from = find_vertex(jmap, node_from, mustfind);
      const auto v_to = find_vertex(jmap, node_to, mustfind);
      if (v_from < 0 || v_to < 0) continue;
      jmap.add(Edge(eid++, road_id, v_from, v_to, maxspeed, gid_start, gid_end));
      if (gen_optim)
      {
          eoptim << road_id << ";x1;" << linestring << ";"
              << node_from << ";" << node_to
              << ";x5;x6;x7;x8;"
              << category << ";x10;x11;x12;"
              << maxspeed1 << ";" << maxspeed2 << std::endl;
      }

      // fill geometry element
      int odistance = 0;
      for (int k = gid_start; k <= gid_end; k++)
      {
          Geom& geom = jmap.geoms[k];
          geom.edge_index = eid - 1;
          int sdistance = LLdistanceLL(geom.lat1, geom.lon1, geom.lat2, geom.lon2);
          geom.distance = sdistance;
          geom.offset = odistance;
          odistance = odistance + sdistance;
      }
  }

  std::cout << "osm read done (" << "# problem speeds:" << nspeeds << ")" << std::endl;

}


int find_vertex(MapCell& jmap, id_g node_id, bool mustfind) {
    for (id_t i = 0; i < jmap.vertices.size(); i++) {
        if (jmap.vertices.at(i).ext_id == node_id)
            return i;
    }
    if (mustfind)
    {
        std::cout << "VERTEXT NOT FOUND\n" << std::endl;
        throw;
        // tbd: assert
    }
    return -1;
}


int get_maxspeed(int id, const MapCell& jmap)
{
    return jmap.edges[id].speed;
}

double distanceLL(double lat1, double lon1, double lat2, double lon2) {
    double theta = lon1 - lon2;
    double dist = std::sin(deg2rad(lat1)) * std::sin(deg2rad(lat2)) +
        std::cos(deg2rad(lat1)) * std::cos(deg2rad(lat2)) *
        std::cos(deg2rad(theta));
    dist = std::acos(dist);
    dist = rad2deg(dist);
    dist = dist * 60 * 1.1515;
    dist = dist * 1.609344;
    return (dist * 1000); // meters
}

double distanceLL2(double lat1, double lon1, double lat2, double lon2) {
    double dlo = deg2rad(lon2 - lon1);
    double dla = deg2rad(lat2 - lat1);
    double la1 = deg2rad(lat1);
    double la2 = deg2rad(lat2);
    double sla = std::sin(dla / 2);
    double slo = std::sin(dlo / 2);
    double a = sla * sla + std::cos(la1) * std::cos(la2) * slo * slo;
    double dist = 2 * std::asin(std::sqrt(a)) * 6372.8;
    return (dist * 1000); // meters
}

double haversine(const GpsPosition& first_coordinate,
    const GpsPosition& second_coordinate) {
    double x1 = first_coordinate.lon;
    double y1 = first_coordinate.lat;
    double x2 = second_coordinate.lon;
    double y2 = second_coordinate.lat;
    return distanceLL2(y1, x1, y2, x2);
}

double haversine_approx(const GpsPosition& first_coordinate,
    const GpsPosition& second_coordinate) {
    double lat1 = first_coordinate.lat;
    double lon1 = first_coordinate.lon;
    double lat2 = second_coordinate.lat;
    double lon2 = second_coordinate.lon;
    double klat = (1.11 * 1000 / 0.01);  // constant coefficients
    double klon = (0.71 * 1000 / 0.01);
    double dlat = lat1 - lat2; // see projection fxp
    double dlon = lon1 - lon2;
    double dx = dlon * klon;   // 8000m max
    double dy = dlat * klat;
    double d2 = dx * dx + dy * dy;
    double d = std::sqrt(d2);
    return d;
}

void unit_test_haversine()
{
    double d1 = haversine(GpsPosition(50.001, 14.500), GpsPosition(50.002, 14.501));
    double d2 = haversine_approx(GpsPosition(50.001, 14.500), GpsPosition(50.002, 14.501));
}

void calc_distances(MapCell& mc) {
    for (int i = 0; i < mc.edges.size(); i++) {
        const Vertex& v1 = mc.vertices.at(mc.edges.at(i).v_from);
        const Vertex& v2 = mc.vertices.at(mc.edges.at(i).v_to);
        GpsPosition p1(v1.lat, v1.lon);
        GpsPosition p2(v2.lat, v2.lon);
        int distance = std::round(haversine(p1, p2));
        mc.edges[i].distance = distance;
        if (distance < 0)
        {
            std::cout << "distance error\n";
        }
    }
}

