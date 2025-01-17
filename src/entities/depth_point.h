#pragma once

#include "common/geom.h"

namespace marine_navi::entities {

struct DepthPoint {
    common::Point Point;
    double Depth;
};

} // namespace entities