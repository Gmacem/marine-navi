#pragma once

#include <string>
#include <optional>

namespace MarineNavi {

enum class ForecastsSource {
  kEsimo = 0,
};

struct Forecast {
  std::string StartedAt;
  std::string FinishedAt;
  ForecastsSource Source;
};

struct ForecastRecord {
  std::string StartedAt;
  std::string Date;
  double Lat;
  double Lon;
  std::optional<double> WaveHeight;
  std::optional<double> SwellHeight;
};

}  // namespace MarineNavi
