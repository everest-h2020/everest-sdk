
#include "components.h"
#include "roadnetwork.h"
#include "structs.h"

// floating-point or fixed-point
//#define FLPFLP
#define ACFXP

#ifndef __BAMBU__
#include <iostream>
#endif

#define __FORCE_INLINE
#include "ac_fixed.h"
#include "ac_math/ac_sqrt.h"
#include "ac_math/ac_abs.h"


#include <limits>
#include <utility>

typedef GpsPosition Coordinate;


// forward declaration
double haversine_top(const GpsPosition& first_coordinate, const GpsPosition& second_coordinate);
std::pair<double, Coordinate> projectPointOnSegment(const Coordinate &source, const Coordinate &target, const Coordinate &coordinate);


void Projection::projection(CandiVector *cv, const GpsVector &gv, const SegVector& sgv, const Edge& e, int node_from, int node_to) {
  // task: fill this cvector (candidate vactor)

  int MAXSIZE = std::min((int)sgv.size(), MAX_SGV_SIZE);
  for (int ig = 0; ig < MAXSIZE; ig++) { // go over all segments of the edge

        const SegItem& seg = sgv[ig];
        // new (projection at the level of geometry)
        GpsPosition p1(seg.lat1, seg.lon1);
        GpsPosition p2(seg.lat2, seg.lon2);

        for (auto ip = 0u; ip < gv.size(); ip++) // go over all FCD points in the trajectory
        {
            const auto& p = gv.at(ip);
            const auto result = projectPointOnSegment(p1, p2, p);
            GpsPosition x(result.second.lat, result.second.lon);
            const auto distance = haversine_top(p, x);

            if (distance < 50)
            {
                auto& candi_list = cv->at(ip);
                int csize = candi_list.size();
                int index = -1;  // if > 0 add or replace candidate list

                // if it is the same edge/road just replace the values
                for (int ic = 0; ic < csize; ic++)
                {
                    const CandiItem& candi = cv->at(ip).at(ic);
                    if (candi.road_id == e.road_id) {
                        if (candi.distance > distance)
                        {
                            index = ic;  // found better
                            break;
                        }
                        else
                        {
                            index = -2; // please ignore
                        }
                    }
                }
                if (index == -2) continue;
                if (index == -1)
                {
                    // if the candidate list is full then replace the worst
                    if (csize == MAX_CANDI_SIZE) {
                        for (int ic = 0; ic < csize; ic++)
                        {
                            //assert(ip >= 0 && ip < MAX_GV_SIZE&& ic >= 0 && ic < MAX_CANDI_SIZE);
                            const CandiItem& candi = cv->at(ip).at(ic);
                            if (candi.distance > distance)
                            {
                                index = ic;  // found replacement
                                break;
                            }
                        }
                    }
                    else // just add it to list
                    {
                        index = candi_list.size();
                    }
                }

                // distances magic
                int pdistance = distance; // projection distance (now calculated)
                int rdistance = e.distance; // road distance (precalculated in map)
                double spos = result.first;   // calculated relative position of the projection on the segment
                int odistance = seg.offset; // offset distance of the current segment (TBD)
                int sdistance = seg.distance; // current segment distance (TBD)
                int ddistance = odistance + sdistance * spos;  // in direction distance
                if (rdistance < ddistance) rdistance = ddistance + 1;  // tbd: fix problem with circular roads, i.e. rdistance = 0
                double pos = (double)ddistance / (double)rdistance;  // relative position of the complete road
                if (pos > 1.0) pos = 1.0;   // hack: safety

                CandiItem candi;
                candi.road_id = e.road_id;
                candi.roadlength = rdistance;
                candi.lat = result.second.lat;
                candi.lon = result.second.lon;
                candi.pos = pos;
                candi.distance = pdistance;
                candi.tis = p.tis;
                candi.speed = p.speed;
                candi.node_from = node_from;
                candi.node_to = node_to;
                if (index == candi_list.size())
                {
                    candi_list.push_back(std::move(candi));
                    //PRINT_DBG("candidate projection distance [%d, %d] %e\n", ip,
                    //    candi_list.size(), distance);
                }
                else if (index != -1)
                {
                    candi_list.at(index) = std::move(candi);
                    //PRINT_DBG("candidate projection distance [%d, %d] %e\n", ip,
                    //    candi_list.size(), distance);
                }
            }
        }

  }
  
}

// Find the closest distance and location between coordinate and the line
// connecting source and target:
//             coordinate
//                 |
//                 |
// source -------- x -------- target.
// returns x as well as the distance between source and x as ratio ([0,1])
//

std::pair<double, Coordinate>
projectPointOnSegment(const Coordinate &source, const Coordinate &target,
                      const Coordinate &coordinate) {

  double proj_lat = 0;
  double proj_lon = 0;
  double clamped_ratio = 0;

#ifdef FLPFLP
  const Coordinate slope_vector{target.lon - source.lon,
                                target.lat - source.lat};
  const Coordinate rel_coordinate{coordinate.lon - source.lon,
                                  coordinate.lat - source.lat};
  // dot product of two un-normed vectors
  const auto unnormed_ratio = slope_vector.lon * rel_coordinate.lon +
                              slope_vector.lat * rel_coordinate.lat;
  // squared length of the slope vector
  const auto squared_length =
      slope_vector.lon * slope_vector.lon + slope_vector.lat * slope_vector.lat;

  if (squared_length < std::numeric_limits<double>::epsilon()) {
    return {0, source};
  }

  const auto normed_ratio = unnormed_ratio / squared_length;
  clamped_ratio = normed_ratio;
  if (clamped_ratio > 1.) {
    clamped_ratio = 1.;
  } else if (clamped_ratio < 0.) {
    clamped_ratio = 0.;
  }

  proj_lat = (1.0 - clamped_ratio) * source.lat + target.lat * clamped_ratio;
  proj_lon = (1.0 - clamped_ratio) * source.lon + target.lon * clamped_ratio;
#endif

#ifdef ACFXP
  ac_fixed<26, 9, true, AC_TRN, AC_SAT> slat = source.lat;   // (200 / 0.00001)
  ac_fixed<26, 9, true, AC_TRN, AC_SAT> slon = source.lon;
  ac_fixed<26, 9, true, AC_TRN, AC_SAT> tlat = target.lat;
  ac_fixed<26, 9, true, AC_TRN, AC_SAT> tlon = target.lon;
  ac_fixed<26, 9, true, AC_TRN, AC_SAT> clat = coordinate.lat;
  ac_fixed<26, 9, true, AC_TRN, AC_SAT> clon = coordinate.lon;
  ac_fixed<14,-4, true, AC_TRN, AC_SAT> slope1 = tlon - slon; // (0.03 / 0.000005) 
  ac_fixed<14,-4, true, AC_TRN, AC_SAT> slope2 = tlat - slat; 
  ac_fixed<14,-4, true, AC_TRN, AC_SAT> rel1 = clon - slon;
  ac_fixed<14,-4, true, AC_TRN, AC_SAT> rel2 = clat - slat;
  ac_fixed<26,-8, true, AC_TRN, AC_SAT> unnormed = slope1 * rel1 + slope2 * rel2; // (1e-3 / 1e-10)
  ac_fixed<26,-8, true, AC_TRN, AC_SAT> squared = slope1 * slope1 + slope2 * slope2;
  float clon_dbg = clon.to_float();
  float slon_dbg = slon.to_float();
  float slope1_dbg = slope1.to_float();
  float slope2_dbg = slope2.to_float();
  float rel1_dbg = rel1.to_float();
  float rel2_dbg = rel2.to_float();
  float unnormed_dbg = unnormed.to_float();
  float squared_dbg = squared.to_float();

  if (squared == 0)
  {
      squared = 0.00001;
  }

  ac_fixed<16, 1, true, AC_TRN, AC_SAT> normed = unnormed / squared;
  float normed_dbg = normed.to_float();

  ac_fixed<16, 2, true, AC_TRN, AC_SAT> clamp = normed;
  ac_fixed<16, 2, true, AC_TRN, AC_SAT> V1 = 1.0;
  if (clamp > 1.) {
      clamp = 1.;
  }
  else if (clamp < 0.) {
      clamp = 0.;
  }

  ac_fixed<26, 9, true, AC_TRN, AC_SAT> proj_x = (V1 - clamp) * slat + tlat * clamp;
  ac_fixed<26, 9, true, AC_TRN, AC_SAT> proj_y = (V1 - clamp) * slon + tlon * clamp;
  proj_lat = proj_x.to_float();
  proj_lon = proj_y.to_float();
  clamped_ratio = clamp.to_float();
#endif

  return {clamped_ratio,
          {
              proj_lat,
              proj_lon,
          }};

}


double haversine_top(const GpsPosition& first_coordinate, const GpsPosition& second_coordinate)
{
    double d;

#ifdef FLPFLP
    d = haversine(first_coordinate, second_coordinate);
#endif

#ifdef ACFXP
    // Central Europe increment unit
    double iLAT = 1.11;
    double iLON = 0.71;

    ac_fixed<26, 9, true, AC_TRN, AC_SAT> lat1 = first_coordinate.lat;  // see projection fxp
    ac_fixed<26, 9, true, AC_TRN, AC_SAT> lon1 = first_coordinate.lon;
    ac_fixed<26, 9, true, AC_TRN, AC_SAT> lat2 = second_coordinate.lat;
    ac_fixed<26, 9, true, AC_TRN, AC_SAT> lon2 = second_coordinate.lon;
    ac_fixed<18, 18, true, AC_TRN, AC_SAT> klat = (iLAT * 1000 / 0.01);  // constant coefficients
    ac_fixed<18, 18, true, AC_TRN, AC_SAT> klon = (iLON * 1000 / 0.01) ;
    ac_fixed<26, 9, true, AC_TRN, AC_SAT> dlat = lat1 - lat2; // see projection fxp
    ac_fixed<26, 9, true, AC_TRN, AC_SAT> dlon = lon1 - lon2;
    ac_fixed<26, 9, false, AC_TRN, AC_SAT> adlat;
    ac_math::ac_abs(dlat, adlat);
    ac_fixed<26, 9, false, AC_TRN, AC_SAT> adlon;
    ac_math::ac_abs(dlon, adlon);
    float klat_dbg = klat.to_float();
    float klon_dbg = klon.to_float();
    ac_fixed<13, 13, false, AC_TRN, AC_SAT> dx = adlon * klon;   // 8000m max
    ac_fixed<13, 13, false, AC_TRN, AC_SAT> dy = adlat * klat;
    float dx_dbg = dx.to_float();
    float dy_dbg = dy.to_float();
    ac_fixed<26, 26, false, AC_TRN, AC_SAT> d2 = dx*dx + dy*dy;
    ac_fixed<13, 13, false, AC_TRN, AC_SAT> da;
    ac_math::ac_sqrt<26, 26, AC_TRN, AC_SAT, 13, 13, AC_TRN, AC_SAT>(d2, da);
    d = da.to_float();

#endif

    return d;
}


void Projection::retrieve_edge_geometry(const MapCell& map_cell, const Edge& e, int& node_from, int& node_to, SegVector* sv )
{
    node_from = map_cell.vertices.at(e.v_from).index;
    node_to = map_cell.vertices.at(e.v_to).index;
    for (int i = e.g_from; i <= e.g_to; i++)
    {
        const Geom& geom = map_cell.geoms[i];
        sv->push_back(std::move(SegItem(geom.lat1, geom.lon1, geom.lat2, geom.lon2, geom.offset, geom.distance)));
    }
}
