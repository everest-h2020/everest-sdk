#ifndef COMPONENTS_XAMPLE_HPP
#define COMPONENTS_XAMPLE_HPP

#include "roadnetwork.h"

class Projection {

public:
  static void projection(CandiVector* cv, const GpsVector& gv, const SegVector& sgv, const Edge& e, int node_from, int node_to);
  static void retrieve_edge_geometry(const MapCell& map_cell, const Edge& e, int& node_from, int& node_to, SegVector* sv);
};

double haversine_top(const GpsPosition& first_coordinate, const GpsPosition& second_coordinate);
double haversine(const GpsPosition &first_coordinate, const GpsPosition &second_coordinate);

#endif
