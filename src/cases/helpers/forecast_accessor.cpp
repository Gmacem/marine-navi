#include "forecast_accessor.h"

#include <algorithm>

namespace marine_navi::cases::helpers {

namespace {

const time_t kTooLate = 6*60*60;

std::unordered_map<int, std::vector<std::tuple<entities::ForecastPoint, double, int>>> GroupForecasts(
    std::vector<std::tuple<entities::ForecastPoint, double, int>> forecasts
) {
  std::unordered_map<int, std::vector<std::tuple<entities::ForecastPoint, double, int>>> result;
  for (const auto& forecast : forecasts) {
    result[std::get<2>(forecast)].push_back(forecast);
  }

  return result;
}

} // namespace

ForecastAccessor::ForecastAccessor(
    std::vector<std::tuple<entities::ForecastPoint, double, int>> forecasts
): forecasts_(GroupForecasts(forecasts)) {}

std::optional<entities::ForecastPoint> ForecastAccessor::GetClosestForecast(int point_id, time_t expected_time) const {
  auto it = forecasts_.find(point_id);
  if (it == forecasts_.end()) {
    return std::nullopt;
  }

  const auto& forecasts = it->second;
  auto nearest_forecast = std::min_element(forecasts.begin(), forecasts.end(), [&expected_time](const auto& lhs, const auto& rhs) {
    return std::abs(std::get<0>(lhs).end_at - expected_time) < std::abs(std::get<0>(rhs).end_at - expected_time);
  });

  if (nearest_forecast == forecasts.end()) {
    return std::nullopt;
  }

  if (std::get<0>(*nearest_forecast).end_at - expected_time > kTooLate) {
    return std::nullopt;
  }

  return std::get<0>(*nearest_forecast);
}


}  // namespace marine_navi::helpers