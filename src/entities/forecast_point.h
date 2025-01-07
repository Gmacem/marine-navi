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

    double GetWaveHeight() const {
        return wave_height.value_or(0) + swell_height.value_or(0);
    }
};

}  // namespace marine_navi::entities