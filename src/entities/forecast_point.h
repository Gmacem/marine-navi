#pragma once

#include <optional>

#include "common/geom.h"

namespace marine_navi::entities {

struct ForecastPoint {
    common::Point point;
    time_t started_at;
    time_t end_at;
    std::optional<double> wave_height;
    std::optional<double> swell_height;
};

}  // namespace marine_navi::entities