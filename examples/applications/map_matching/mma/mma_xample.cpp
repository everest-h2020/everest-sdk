
#include "structs.h"
#include "roadnetwork.h"
#include "fcd.h"
#include "components.h"
#include <vector>
#include <chrono>
#include <fstream>
#include <iostream>

// example execution
// ./mma_xample -root .. -map data/map_o3 -fcd data/fcd_test1.csv -out output.csv


#ifndef MAX_MATCH
#define MAX_MATCH 5
#endif

#ifdef __BAMBU_SIM__
#include <mdpi/mdpi_user.h>
#endif

#pragma HLS interface port = cv mode = m_axi offset = direct bundle = common
#pragma HLS interface port = gv mode = m_axi offset = direct bundle = common
#pragma HLS interface port = map_cell mode = m_axi offset = direct bundle = common
extern "C" __attribute_noinline__ 
void kernel_projection(CandiVector* cv, const GpsVector& gv,
    const MapCell& map_cell) {

    cv->resize(gv.size());

    for (auto im = 0u; im < map_cell.edges.size(); im++)
    {
        const Edge& e = map_cell.edges.at(im);
        SegVector sv = {};
        int node_from = 0;
        int node_to = 0;
        Projection::retrieve_edge_geometry(map_cell, e, node_from, node_to, &sv);
        Projection::projection(cv, gv, sv, e, node_from, node_to);
    }
}


GpsVector read_vector(std::ifstream& f) {
    GpsVector gv;

    double lat, lon;
    while (f >> lat >> lon) {
        if (lat == 0 && lon == 0) {
            break;
        }
        gv.push_back(GpsPosition(lat, lon));
    }

    return gv;
}


int find_cell(const std::vector<GpsRectangle>& cells, const GpsPosition& sample) {
    for (int i = 0; i < cells.size(); ++i) {
        if (sample.lat > cells[i].lat1 && sample.lat < cells[i].lat2 && sample.lon > cells[i].lon1 && sample.lon < cells[i].lon2)
            return i;
    }
    return -1;
}

void write_vector_out(std::ofstream& f, const CandiVector& cv, int vnumber) {
    for (int i = 0; i < cv.size(); ++i) {
        const auto cl = cv.at(i);
        f << vnumber << ";";
        for (int j = 0; j < MAX_CANDI_SIZE; j++)
        {
            int road_id = -1;
            if (j < cl.size()) road_id = cl.at(j).road_id;
            f << road_id << ";";
        }
        f << "\n";
    }
}



static MapCell mapcell[100];

int main(int argc, const char* argv[])
{
    std::string DDIR = "data";

    const int dinformat = 2;  // 1-direct gpsvector, 2-fcd spec format, 3-fcd verbose format
    const int mapformat = 3;  // 1-sygic, 2-osm, 3-osm2

    // preconfigured datasources
    std::string MAPINPUT = DDIR + "/osm_praha_v2/optim3";
    std::string DATAINPUT = DDIR + "/fcd/5K_GPS_210920.csv";
    std::string map_nodes = MAPINPUT + "/_nodes.csv";
    std::string map_edges = MAPINPUT + "/_edges.csv";
    std::string DATAOUTPUT = DDIR + "/output.txt";

    // reading command line
    int j = 1;
    while (j < argc)
    {
        if (std::string(argv[j]) == "-root") {
            j++; 
            DDIR = std::string(argv[j]);
        }
        if (std::string(argv[j]) == "-map") { 
            j++; 
            MAPINPUT = DDIR + "/" + std::string(argv[j]);
            map_nodes = MAPINPUT + "/_nodes.csv";
            map_edges = MAPINPUT + "/_edges.csv";
        }
        if (std::string(argv[j]) == "-fcd") {
            j++;
            DATAINPUT = DDIR + "/" + std::string(argv[j]);
        }
        if (std::string(argv[j]) == "-out") {
            j++;
            DATAOUTPUT = DDIR + "/" + std::string(argv[j]);
        }
        j++;
    }

    // this is the config
    std::cout << "settings ..." << std::endl;
    std::cout << "ROOTDIR = " << DDIR << std::endl;
    std::cout << "MAPDATA = {" << map_nodes << "," << map_edges << "}" << std::endl;
    std::cout << "FCD     = " << DATAINPUT << std::endl;

    // input/output streams
    std::ifstream datastore1(DATAINPUT);
    std::ofstream datastore2(DATAOUTPUT);

    // map boundary configuration (Prague)
    GpsRectangle fcdbox(49.96, 14.23, 50.16, 14.65);

    // fcd preprocessing (reading and filtering)
    std::cout << "... reading fcd " << std::endl;
    FCD fcdproc(datastore1, fcdbox);
    fcdproc.initialize(dinformat);
    std::cout << "... done fcd size= " << fcdproc.fcd_size() << std::endl;

    // reading map split into NxN
    int NN = 5;
    double lat0 = fcdbox.lat1;
    double lon0 = fcdbox.lon1;
    double latd = (fcdbox.lat2 - lat0) / NN;
    double lond = (fcdbox.lon2 - lon0) / NN;

    // configure map
    std::vector <GpsRectangle> mapbox = {};
    for (int z1 = 0; z1 < NN; z1++) {
        for (int z2 = 0; z2 < NN; z2++) {
            double lat1 = lat0 + z1 * latd;
            double lon1 = lon0 + z2 * lond;
            double lat2 = lat1 + latd;
            double lon2 = lon1 + lond;
            mapbox.push_back(std::move(GpsRectangle(lat1, lon1, lat2, lon2)));
        }
    }

    // map reading
    for (int i = 0; i < mapbox.size(); i++) {
        std::cout << "reading mapcell " << i << std::endl;
        read_map_osm(mapformat, mapcell[i], map_nodes, map_edges, mapbox[i]);
        calc_distances(mapcell[i]);
        std::cout << "  vertices: " << mapcell[i].vertices.size() << std::endl;
        std::cout << "  edges   : " << mapcell[i].edges.size() << std::endl;
    }

    // ---------------------------------------------------
    // processing vectors one by one
    using pmetric = std::chrono::milliseconds;
    auto start = std::chrono::high_resolution_clock::now();
    int count_valid = 0;
    for (int i = 0; i < MAX_MATCH; ++i)
    {
        int vnumber = i + 1;

        GpsVector gv;
        std::string vid;
        gv = fcdproc.get_next_vector(vid);    // from fcd data

        if (gv.empty()) {
            break;
        }

        // map cell indices
        const auto& sample = gv.at((int)(gv.size() / 2));
        int index = find_cell(mapbox, sample);

        std::cout << "... executing vector " << vnumber << " (id:" << vid << " size:" << gv.size()
                  << " tis : " << gv[0].tis << " cell : " << index << ")" << std::endl;

        CandiVector cv = {};

        #ifdef __BAMBU_SIM__
        m_param_alloc(0, sizeof(CandiVector));
        #endif
        kernel_projection(&cv, gv, mapcell[index]);

        bool projection_valid = true;
        for (int i = 0; i < cv.size(); i++)
        {
            if (cv.at(i).size() == 0) projection_valid = false;
        }
        if (!projection_valid) {
            count_valid++;
            write_vector_out(datastore2, cv, vnumber);
        }

    }
    // end vector processing
    // ---------------------------------------------------

    auto finish = std::chrono::high_resolution_clock::now();
    std::cout << "vectors valid: " << count_valid << "\n";
    std::cout << "EXECUTION: " << std::chrono::duration_cast<pmetric>(finish - start).count() << " milliseconds\n";
    std::cout << "EOT\n";

    return 0;
}

