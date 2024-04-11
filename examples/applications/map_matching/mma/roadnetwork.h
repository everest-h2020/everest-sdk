#ifndef MAPRN_HPP
#define MAPRN_HPP

#include "structs.h"

#ifndef __BAMBU__
#include <string>

#ifdef STATIC
#include <iostream>
#endif
#endif

#ifndef MAX_EDGES
#define MAX_EDGES 2000
#endif

#ifndef MAX_EDGES_PER_VERTEX
#define MAX_EDGES_PER_VERTEX 16
#endif

#ifndef MAX_VERTICES
#define MAX_VERTICES 10000
#endif

#ifndef MAX_GEOMS
#define MAX_GEOMS 100000
#endif


struct Vertex {
  int index;      // array index (auxiliary for Dijkstra)
  id_g ext_id;    // external id
  double lat;
  double lon;
  utils::vector_t<id_t, MAX_EDGES_PER_VERTEX> edges;
  Vertex(int _index, id_g _id, const double lat, const double lon)
      : index(_index), ext_id(_id), lat(lat), lon(lon) {}
};

struct Edge {
  int road_id;      // internal id ( instead of large numbers of external id )
  id_g ext_id;      // external id
  id_t v_from;      // index pointer in Vertex array
  id_t v_to;        // index pointer to Vertex array
  int distance;     // meters
  int speed;        // km/h
  id_t g_from;      // index pointer to geometry array
  id_t g_to;        // index pointer to geometry array
  Edge(id_t _index, id_g _id, id_t _v_from, id_t _v_to, int _speed, id_t _g_from, id_t _g_to)
      : road_id(_index), ext_id(_id), v_from(_v_from), v_to(_v_to), speed(_speed), g_from(_g_from), g_to(_g_to), distance(-1) {}
};

struct Geom {
  int index; 
  int edge_index;  // index pointer to Edge array
  double lat1;
  double lon1;
  double lat2;
  double lon2;
  int distance;  // meters
  int offset;    // meters
  Geom(int _index, id_t _edge_index, double _lat1, double _lon1, double _lat2, double _lon2)
      : index(_index), edge_index(_edge_index), lat1(_lat1), lon1(_lon1), lat2(_lat2), lon2(_lon2), distance(-1), offset(0) {}
};

struct MapCell {
  // maximum 1000 vertices and 2000 edges
  utils::vector_t<Vertex, MAX_VERTICES> vertices;
  utils::vector_t<Edge, MAX_EDGES> edges;
  utils::vector_t<Geom, MAX_GEOMS> geoms;

#ifndef __BAMBU__
  id_t add(Vertex &&v) {
#ifdef STATIC
    if (vertices.full()) {
      std::cerr << "Map cell max vertices reached" << std::endl;
      throw;
    }
#endif
    vertices.push_back(v);
    return vertices.size() - size_t(1);
  }

  id_t add(Edge &&e) {
    edges.push_back(e);
    id_t e_id = edges.size() - size_t(1);
#if STATIC
    if (vertices.at(e.v_from).edges.full()) {
      std::cerr << "Vertex max edges reached" << std::endl;
      throw;
    }
#endif
    vertices.at(e.v_from).edges.push_back(e_id);
    return e_id;
  }

  id_t add(Geom&& g) {
#if STATIC
      if (geoms.full()) {
          std::cerr << "Map cell max geoms reached" << std::endl;
          throw;
      }
#endif
      geoms.push_back(g);
      return geoms.size() - size_t(1);
  }

#endif
};

#ifndef __BAMBU__
void read_map(MapCell &mapcell, const std::string &filename);
void read_map_osm(int mf, MapCell& mapcell, const std::string& vfilename, const std::string& efilename, const GpsRectangle& box);
void calc_distances(MapCell &mc);
void vertex_indexation(MapCell &mc);
int find_vertex(MapCell& jmap, id_g node_id, bool mustfind);
id_g find_ext_road_id(int id, const MapCell& jmap);
int get_maxspeed(int id, const MapCell& jmap);
#endif

#endif