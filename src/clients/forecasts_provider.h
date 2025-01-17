#pragma once

#include "entities/weather_forecast.h"

#include <vector>

namespace marine_navi::clients {

class IForecastsProvider {
public:
  virtual void LoadForecasts() = 0;
  virtual entities::Forecast GetForecast() = 0;
  virtual std::vector<entities::ForecastRecord> GetRecords() = 0;
  virtual ~IForecastsProvider() = default;
};
}  // namespace marine_navi::clients
