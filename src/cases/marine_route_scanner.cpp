#include "marine_route_scanner.h"

#include "common/marine_math.h"
#include "entities/depth_point.h"

namespace marine_navi::cases {

namespace {


std::vector<entities::RoutePoint> GetRoutePoints(const std::shared_ptr<entities::Route>& route, double step_dist) {
  std::vector<entities::RoutePoint> result;
  
  double total = route->GetDistance();

  for (size_t i = 0; i * step_dist < total; ++i) {
    result.emplace_back(route->GetPointFromStart(i * step_dist));
  }
  return result;
}

[[maybe_unused]] bool IsPointOnRoute(const common::Segment& segment, const common::Point point, double alpha) {
  const auto direction = segment.End - segment.Start;
  const auto point_vec = point - segment.Start;

  return common::IsInsideOfAngle(direction, point_vec, direction.Rotate(alpha)) ||
         common::IsInsideOfAngle(direction, point_vec, direction.Rotate(-alpha));
}

double GetSpeed(const RouteData& route_data, const double wave_height) {
  if (
      !route_data.DangerHeight.has_value() ||
      !route_data.EnginePower.has_value() ||
      !route_data.Displacement.has_value() ||
      !route_data.Length.has_value() ||
      !route_data.Fullness.has_value() ||
      !route_data.ShipDraft.has_value()
  ) {
    return route_data.Speed.value();
  }
  auto r = common::CalculateVelocityRatio(
    route_data.EnginePower.value(),
    route_data.Displacement.value(),
    route_data.Length.value(),
    route_data.Fullness.value(),
    wave_height
  );

  return r * route_data.Speed.value();
}

} // namespace

MarineRouteScanner::MarineRouteScanner(std::shared_ptr<clients::DbClient> dbClient)
    : mutex_(), route_data_(), show_(false), db_client_(dbClient) {}

void MarineRouteScanner::SetPathData(const RouteData& pathData) {
  std::lock_guard lock(mutex_);
  route_data_ = pathData;
}

const RouteData& MarineRouteScanner::GetPathData() {
  std::lock_guard lock(mutex_);
  return route_data_;
}

void MarineRouteScanner::SetShow(bool show) {
  std::lock_guard lock(mutex_);
  show_ = show;
}

bool MarineRouteScanner::IsShow() {
  std::lock_guard lock(mutex_);
  return show_;
}

bool MarineRouteScanner::CheckLandIntersection(const Point& p1,
                                               const Point& p2) const {
  static constexpr double EPS = 1e-5;

  Point minCorner{std::min(p1.Lat, p2.Lat), std::min(p1.Lon, p2.Lon)};
  Point maxCorner{std::max(p1.Lat, p2.Lat), std::max(p1.Lon, p2.Lon)};
  minCorner.Lat -= EPS;
  minCorner.Lon -= EPS;
  maxCorner.Lat += EPS;
  maxCorner.Lon += EPS;

  return PlugIn_GSHHS_CrossesLand(minCorner.Lat, minCorner.Lon, maxCorner.Lat,
                                  maxCorner.Lon);
}

std::optional<entities::diagnostic::RouteValidateDiagnostic> MarineRouteScanner::GetDiagnostic() {
  std::lock_guard lock(mutex_);
  return diagnostic_;
}

void MarineRouteScanner::CrossDetect() {
  std::lock_guard lock(mutex_);
  fprintf(stderr, "Cross detect\n");

  try {
    diagnostic_ = DoCrossDetect();
  } catch (std::exception& ex) {
    fprintf(stderr, "Failed detect impl %s\n", ex.what());
    diagnostic_ = std::nullopt;  // TODO: signal about it
  }
}

std::vector<MarineRouteScanner::RoutePointWithForecast> MarineRouteScanner::GetRouteInfo() const {
  const double kForecastStepMeters = 10000;
  const double kDangerousDistanceRad = 0.1;
  const int64_t kForecastValidTime = 6*60*60; // maximum time the forecast is valid
  const double kMidPointDistance = 100;
  const auto route_points = GetRoutePoints(route_data_.Route, kForecastStepMeters);
  std::vector<common::Point> points;
  std::transform(route_points.begin(), route_points.end(), std::back_inserter(points),
                 [](const entities::RoutePoint& route_point) { return route_point.point; });
  auto forecasts = db_client_->SelectNearestForecasts(points, kDangerousDistanceRad, route_data_.DepartTime);
  std::vector<std::vector<std::pair<entities::ForecastPoint, double>>> grouped(route_points.size());
  for(const auto& forecast : forecasts) {
    int id = std::get<2>(forecast);
    grouped[id].emplace_back(std::get<0>(forecast), std::get<1>(forecast));
  }
  time_t cur_time = route_data_.DepartTime;
  double cur_speed = route_data_.Speed.value();

  std::vector<MarineRouteScanner::RoutePointWithForecast> result;

  for(size_t i = 0; i < route_points.size(); ++i) {
    const auto& route_point = route_points[i];
    const auto& forecasts = grouped[i];
    time_t forecast_delta_time = std::numeric_limits<time_t>::max();
    std::optional<entities::ForecastPoint> nearest_forecast;
    for(const auto& forecast : forecasts) {
      time_t delta = cur_time - forecast.first.end_at;
      if (forecast.first.end_at <= cur_time 
          && forecast_delta_time > delta
          && delta < kForecastValidTime) { 
        forecast_delta_time = delta;
        nearest_forecast = forecast.first;
      }
    }

    if (nearest_forecast.has_value()) {
      cur_speed = GetSpeed(route_data_, nearest_forecast->GetWaveHeight());
    }

    // add mid point for depth check
    if (i > 0) {
      const auto& previous_point = route_points[i - 1];
      for(int j = 1; previous_point.distance_from_start_route + kMidPointDistance * j < route_point.distance_from_start_route; ++j) {
        double delta = kMidPointDistance * j;
        double distance_from_start = previous_point.distance_from_start_route + delta;
        const auto mid_point = route_data_.Route->GetPointFromStart(distance_from_start);
        const time_t delta_time = delta / cur_speed;

        result.push_back(RoutePointWithForecast{
          .route_point = mid_point,
          .nearest_forecast = nearest_forecast,
          .speed = cur_speed,
          .expected_time = cur_time + delta_time
        });
      }
      cur_time += (route_point.distance_from_start_route - previous_point.distance_from_start_route) / cur_speed;
    }
    result.push_back(RoutePointWithForecast{
      .route_point = route_point,
      .nearest_forecast = nearest_forecast,
      .speed = cur_speed,
      .expected_time = cur_time,
    });
  }

  return result;
}

std::vector<entities::diagnostic::DiagnosticHazardPoint> MarineRouteScanner::GetForecastDiagnostic(
  const std::vector<RoutePointWithForecast>& route, const time_t check_time
) const {
  std::vector<entities::diagnostic::DiagnosticHazardPoint> result;

  for(const auto& route_point : route) {
    const auto& nearest_forecast = route_point.nearest_forecast;

    if (!nearest_forecast.has_value()) {
      continue;
    }

    if (nearest_forecast->GetWaveHeight() > route_data_.DangerHeight.value()) {
      result.push_back(entities::diagnostic::MakeHighWavesHazardPoint(
        route_point.route_point.point,
        check_time,
        route_point.expected_time,
        nearest_forecast->GetWaveHeight())
      );
    }
  }
  return result;
}

std::vector<entities::diagnostic::DiagnosticHazardPoint> MarineRouteScanner::GetDepthDiagnostic(
  const std::vector<RoutePointWithForecast>& route,
  const time_t check_time
) const {
  const size_t kLimitReturnSize = 100;

  std::optional<entities::DepthGrid> grid;
  if (route_data_.PathToDepthFile.has_value() &&
      route_data_.ShipDraft.has_value()) {
    grid = entities::DepthGrid(route_data_.PathToDepthFile.value());
  } else {
    wxLogWarning("depth grid or ship draft is not provided");
    return {};
  }

  std::vector<entities::diagnostic::DiagnosticHazardPoint> result;

  for(const auto& route_point : route) {
    const auto depth_point = grid->GetNearestDepthPoint(route_point.route_point.point);
    if (depth_point.has_value() && -depth_point->Depth < route_data_.ShipDraft.value()) {
      result.push_back(entities::diagnostic::MakeDepthHazardPoint(
        depth_point->Point,
        check_time,
        route_point.expected_time,
        depth_point->Depth
      ));
    }

    if (result.size() > kLimitReturnSize) {
      break;
    }
  }

  return result;
}

entities::diagnostic::RouteValidateDiagnostic MarineRouteScanner::DoCrossDetect() const {
  const auto route_info = GetRouteInfo();
  const time_t check_time = common::GetCurrentTime();
  const auto diagnostic_forecast = GetForecastDiagnostic(route_info, check_time);
  const auto diagnostic_depth = GetDepthDiagnostic(route_info, check_time);

  if (diagnostic_forecast.empty() && diagnostic_depth.empty()) { 
    return entities::diagnostic::RouteValidateDiagnostic{
      .result = entities::diagnostic::RouteValidateDiagnostic::DiagnosticResultType::kOk,
      .hazard_points = {}
    };
  }

  auto hazard_points = diagnostic_depth;
  hazard_points.insert(hazard_points.end(), diagnostic_forecast.begin(), diagnostic_forecast.end());

  return entities::diagnostic::RouteValidateDiagnostic{
    .result = entities::diagnostic::RouteValidateDiagnostic::DiagnosticResultType::kWarning,
    .hazard_points = hazard_points
  };
}

}  // namespace marine_navi::cases
