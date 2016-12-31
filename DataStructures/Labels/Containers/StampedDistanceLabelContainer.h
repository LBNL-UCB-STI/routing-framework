#pragma once

#include <algorithm>
#include <cassert>
#include <type_traits>
#include <vector>

#include "Tools/Concurrent/NonAtomic.h"
#include "Tools/Concurrent/RelaxedAtomic.h"
#include "Tools/CompilerSpecific.h"
#include "Tools/Constants.h"

// A container maintaining distance labels. It stores a global clock and a timestamp for each
// distance label. The timestamp indicates whether a distance label has a valid value or not.
template <typename DistanceLabelT>
class StampedDistanceLabelContainer {
 public:
  // Constructs a distance label container using timestamps.
  explicit StampedDistanceLabelContainer(const int numVertices)
      : distanceLabels(numVertices), timestamps(numVertices), clock(0) {
    std::fill(timestamps.begin(), timestamps.end(), 0);
  }

  // Initializes all distance labels to infinity.
  void init() {
    ++clock;
    if (UNLIKELY(clock < 0)) {
      // Clock overflow occurred. Extremely unlikely.
      std::fill(timestamps.begin(), timestamps.end(), 0);
      clock = 1;
    }
  }

  // Returns a reference to the distance label of v.
  DistanceLabelT& operator[](const int v) {
    assert(v >= 0); assert(v < distanceLabels.size());
    if (timestamps[v] != clock) {
      assert(timestamps[v] < clock);
      distanceLabels[v] = INFTY;
      timestamps[v].store(clock, std::memory_order_release);
    }
    return distanceLabels[v];
  }

  // Returns the distance label of v.
  DistanceLabelT get(const int v) const {
    return timestamps[v].load(std::memory_order_acquire) == clock ? distanceLabels[v] : INFTY;
  }

 private:
  using Timestamp =
      std::conditional_t<DistanceLabelT::parallel, RelaxedAtomic<int>, NonAtomic<int>>;

  std::vector<DistanceLabelT> distanceLabels; // The distance labels of the vertices.
  std::vector<Timestamp> timestamps;          // The timestamps indicating whether a label is valid.
  int clock;                                  // The global clock.
};
