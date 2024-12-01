#pragma once

#include "entities/WeatherForecast.h"

#include <vector>

namespace MarineNavi {

class IForecastsProvider {
public:
  virtual void LoadForecasts() = 0;
  virtual Forecast GetForecast() = 0;
  virtual std::vector<ForecastRecord> GetRecords() = 0;
  virtual ~IForecastsProvider() = default;
};
}  // namespace MarineNavi
