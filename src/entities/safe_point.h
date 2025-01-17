#pragma once

#include "common/geom.h"

namespace marine_navi::entities {

struct SafePoint {
    common::Point Point;
    std::string Name;
};

} // namespace marine_navi::entities