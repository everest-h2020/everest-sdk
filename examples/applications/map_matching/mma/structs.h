#ifndef STRUCTS_HPP
#define STRUCTS_HPP

#include "utils.hpp"

#ifndef MAX_GV_SIZE
#define MAX_GV_SIZE 10
#endif
#ifndef MAX_RSV_SIZE
#define MAX_RSV_SIZE 30
#endif
#ifndef MAX_CANDI_SIZE
#define MAX_CANDI_SIZE 5
#endif

#ifndef MAX_ROAD_PATH
#define MAX_ROAD_PATH 5
#endif

#ifndef MAX_SGV_SIZE
#define MAX_SGV_SIZE 20
#endif

typedef unsigned int id_t;
typedef long long id_g;

struct GpsPosition {
  double lat;
  double lon;
  int tis;  // time of a day in seconds
  int speed; // in km/h

  GpsPosition() : lat(0.0), lon(0), tis(0), speed(0) {}
  GpsPosition(const GpsPosition &other) : lat(other.lat), lon(other.lon), tis(other.tis), speed(other.speed) {}
  GpsPosition(const double _lat, const double _lon) : lat(_lat), lon(_lon), tis(0), speed(0) {}
  GpsPosition(const double _lat, const double _lon, int _tis, int _speed) : lat(_lat), lon(_lon), tis(_tis), speed(_speed) {}
};

struct GpsRectangle {
    double lat1; // bottom left
    double lon1;
    double lat2; // top right
    double lon2;
    GpsRectangle() : lat1(0), lon1(0), lat2(0), lon2(0) {}
    GpsRectangle(const GpsRectangle& other) : lat1(other.lat1), lon1(other.lon1), lat2(other.lat2), lon2(other.lon2) {}
    GpsRectangle(const double _lat1, const double _lon1, const double _lat2, const double _lon2)
        : lat1(_lat1), lon1(_lon1), lat2(_lat2), lon2(_lon2) {}
};

using GpsVector = utils::vector_t<GpsPosition, MAX_GV_SIZE>;

struct RoadSegment
{
    int node_id;     // = vertex_id
    int road_id;     // = edge_id
    int roadlength;  // = edge distance
    RoadSegment(int _node_id, int _road_id, int _roadlength)
        : node_id(_node_id), road_id(_road_id), roadlength(_roadlength) {}
    RoadSegment()
        : node_id(-1), road_id(-1), roadlength(-1) {}
};

using RoadPath = utils::vector_t<RoadSegment, MAX_ROAD_PATH>;

struct CandiItem {
  double lat;
  double lon;
  int road_id;
  int roadlength;   // length of the road segment
  double pos;       // position on the roadlength from start (0 to 1.0)
  int distance;     // distance of projection to the road
  int speed;        // measured speed
  int tis;          // time of a day in seconds from noisy GPS point
  int node_from;    // array index (not vertex id)
  int node_to;      // array index (not vertex_id)
  int vertex_from;  // used for debugging
  int vertex_to;    // used for debugging

  CandiItem()
      : lat(0), lon(0), road_id(-1), distance(-1), roadlength(-1), pos(-1), tis(-1), 
        node_from(-1), node_to(-1), vertex_from(-1), vertex_to(-1), speed(-1) {}
};

using CandiList = utils::vector_t<CandiItem, MAX_CANDI_SIZE>;

using CandiVector = utils::vector_t<CandiList, MAX_GV_SIZE>;

struct SegItem {
    double lat1;
    double lon1;
    double lat2;
    double lon2;
    double offset;
    double distance;
    SegItem(double _lat1, double _lon1, double _lat2, double _lon2, double _offset, double _distance)
        : lat1(_lat1), lon1(_lon1), lat2(_lat2), lon2(_lon2), offset(_offset), distance(_distance) {}
};

using SegVector = utils::vector_t<SegItem, MAX_SGV_SIZE>;


struct RoadSpeed {
  int road_id;    // map index
  int roadlength; // meters
  int node_from;  // array index
  int node_to;    // array index
  int tis;        // -1 on the interpolated roads
  double pos;     // position of tis referring to segment start; -1 on the interpolated roads
  double speed0;  // measured speed
  double speed;   // calculated speed 

  RoadSpeed() : road_id(-1), node_from(-1), node_to(-1), roadlength(-1), tis(-1), pos(-1), speed0(-1), speed(-1) {}
  RoadSpeed(int _road_id, int _node_from, int _node_to, int _roadlength, int _tis, double _pos, int _speed0, int _speed)
      : road_id(_road_id), node_from(_node_from), node_to(_node_to), roadlength(_roadlength), 
        tis(_tis), pos(_pos), speed0(_speed0), speed(_speed) {}
};

using RoadSpeedVector = utils::vector_t<RoadSpeed, MAX_RSV_SIZE>;

struct TrellisItem {
  int road_id;
  double proba;
  double distance;
  unsigned parent;
  bool reachable;

  TrellisItem()
      : road_id(0), proba(0.0), distance(0.0), parent(0), reachable(false) {}
};

struct Trellis {
  unsigned int t_length;
  TrellisItem treli[MAX_GV_SIZE][MAX_CANDI_SIZE];

  Trellis() : t_length(0) {}
};

#endif
