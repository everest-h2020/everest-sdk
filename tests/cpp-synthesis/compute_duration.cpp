#include "etl/include/etl/vector.h"

#define SHIFT(lfsr, poly_mask)                                                 \
  lfsr >>= 1;                                                                  \
  lfsr ^= ((((lfsr & 1) == 1) << 31) >> 31) & poly_mask

class LFSR16 {
private:
  const unsigned int _poly_mask_32;
  const unsigned int _poly_mask_31;
  unsigned int _lfsr32;
  unsigned int _lfsr31;

public:
  LFSR16(unsigned int lfsr32_init, unsigned int poly_mask_32,
         unsigned int lfsr31_init, unsigned int poly_mask_31)
      : _lfsr32(lfsr32_init), _lfsr31(lfsr31_init), _poly_mask_32(poly_mask_32),
        _poly_mask_31(poly_mask_31) {}

  inline unsigned short next() {
    SHIFT(_lfsr32, _poly_mask_32);
    SHIFT(_lfsr32, _poly_mask_32);
    SHIFT(_lfsr31, _poly_mask_31);
    return _lfsr32 ^ _lfsr31;
  }

  void seed(unsigned int lfsr32_init, unsigned int lfsr31_init) {
    _lfsr32 = lfsr32_init;
    _lfsr31 = lfsr31_init;
  }
};


#include <limits>


#ifndef MAX_SIZE_ID
#define MAX_SIZE_ID 32ULL
#endif

#ifndef MAX_SIZE_SEGMENTS
#define MAX_SIZE_SEGMENTS 160ULL
#endif


// A single segment of a road.
struct Segment {
  double length;
  double speed;    
};

template <typename T, const unsigned long long MAX_SIZE>
using vector_t = etl::vector_storage<T, MAX_SIZE>;

// Probability profile for a single segment.
// This profile is sampled to determine the Level of Service
// (how easy is to go through the segment).
struct SegmentTimeProfile {
  double values[4];
  double cum_probs[4];
};

// Wrapper struct that contains segment data and its probability profile.
struct EnrichedSegment {
  Segment segment;
  SegmentTimeProfile profiles[672];
};

// Single route which will be sampled using Monte Carlo to determine how long
// would it take to go through it.
struct Route {
  // Duration of an atomic movement of a car on a segment.
  double frequency_seconds;
  vector_t<EnrichedSegment, MAX_SIZE_SEGMENTS> segments;
};

// Position of a car on some route.
struct RoutePosition {
  // Index of a specific segment on which the car is currently located.
  unsigned long long segment_index;
  // Number in the range [0.0, 1.0] which determines how far is the car along
  // the segment.
  double progress;
};

unsigned long long ComputeDuration(const Route *const route,
                const RoutePosition *const start,
                const unsigned long long departure,
                const unsigned long long seed) 
{
  LFSR16 lfsr(0xABCDE, 0xB4BCD35C, 0x23456789, 0x7A5BC2E3);
  lfsr.seed(seed >> 32, seed);

  // Calculates an index of a 15-minute interval of a week from the given number
  // of seconds. The index is in the range [0, 671].
  const auto datetime_to_minute_index =
      [](const unsigned long long time) -> unsigned long long {
    const auto within_week = time % (7ULL * 24 * 3600 * 1000);
    return within_week / (15ULL * 60 * 1000);
  };
  // Sample the Level of service from the given profile.
  const auto find_los_by_cumprob =
      [](const unsigned short random_number,
         const SegmentTimeProfile &profile)
      -> double {
    for (unsigned long long index = 0; index < 4; index++) {
      const auto cum_prob = profile.cum_probs[index];
      if (random_number < static_cast<unsigned short>(
                              cum_prob * std::numeric_limits<unsigned short>::max()))
      {
        return profile.values[index];
      }
    }
    return profile.values[3];
  };

  const auto advance_route = [](Route *const route,
                                RoutePosition &position,
                                unsigned long long &time, double los) {
    const auto &segment = route->segments[position.segment_index];
    const auto speed = segment.segment.speed * los;
    if (speed != 0.0) {
      const auto frequency = route->frequency_seconds;
      const auto elapsed = position.progress + frequency * speed;

      if (elapsed < segment.segment.length) {
        time += frequency;
        position.progress = elapsed;
      } else {
        const auto rest_dist = segment.segment.length - position.progress;
        const auto elapsed_time = rest_dist / speed;

        time += elapsed_time;
        position.segment_index += 1;
        position.progress = 0.0;
      }
    }
  };

  unsigned long long time = departure;
  RoutePosition position = *start;

  while (position.segment_index < route->segments.size()) {
    const auto &segment = route->segments[position.segment_index];
    const auto rnd_val =
        lfsr.next();
    const auto dtm = datetime_to_minute_index(time);
    const auto &profile = segment.profiles[dtm];
    const auto los = find_los_by_cumprob(rnd_val, profile) / 100.0;
  }

  return time - departure;
}

#pragma HLS_interface port = duration mode = m_axi offset = direct bundle = common
#pragma HLS_interface port = route mode = m_axi offset = direct bundle = common
#pragma HLS_interface port = start mode = m_axi offset = direct bundle = common
void kernel_compute_duration(unsigned long long *duration,
                             const Route *const route,
                             const RoutePosition *const start,
                             const unsigned long long departure,
                             const unsigned long long seed) {
  const auto ret =
      ComputeDuration(route, start, departure, seed);
  *duration = ret;
}