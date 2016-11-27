#pragma once

#include <cassert>

#include "DataStructures/Graph/Attributes/AbstractAttribute.h"

// Road categories defined by the XATF file format.
enum class XatfRoadCategory {
  MOTORWAY_FAST        = 1,
  MOTORWAY_MEDIUM      = 2,
  MOTORWAY_SLOW        = 3,
  NATIONAL_ROAD_FAST   = 4,
  NATIONAL_ROAD_MEDIUM = 5,
  NATIONAL_ROAD_SLOW   = 6,
  REGIONAL_ROAD_FAST   = 7,
  REGIONAL_ROAD_MEDIUM = 8,
  REGIONAL_ROAD_SLOW   = 9,
  URBAN_STREET_FAST    = 10,
  URBAN_STREET_MEDIUM  = 11,
  URBAN_STREET_SLOW    = 12,
  FERRY                = 13,
  UNUSED               = 14,
  FOREST_ROAD          = 15,
};

// An attribute associating an XATF road category with each edge of a graph.
class XatfRoadCategoryAttribute : public AbstractAttribute<XatfRoadCategory> {
 public:
  static constexpr Type DEFAULT_VALUE = XatfRoadCategory::UNUSED; // The attribute's default value.
  static constexpr const char* NAME   = "xatf_road_category";     // The attribute's unique name.

  // Returns the XATF road category of edge e.
  Type xatfRoadCategory(const int e) const {
    assert(e >= 0); assert(e < values.size());
    return values[e];
  }

  // Sets the XATF road category of edge e to val.
  void setXatfRoadCategory(const int e, const Type val) {
    assert(e >= 0); assert(e < values.size());
    values[e] = val;
  }
};