#pragma once

#include <memory>
#include <unordered_map>

#include "entities/forecast_point.h"

namespace marine_navi::cases::helpers {


class ForecastAccessor {
public:
    ForecastAccessor(
        std::vector<std::tuple<entities::ForecastPoint, double, int>> forecasts
    );

    std::optional<entities::ForecastPoint> GetClosestForecast(int point_id, time_t expected_time) const;

private:
    const std::unordered_map<int, std::vector<std::tuple<entities::ForecastPoint, double, int>>> forecasts_;
};


}  // namespace marine_navi::cases::helpers