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

std::tuple<common::Polygon, common::Polygon> MakeDepthCheckPolygon(const common::Segment& segment, double alpha) {
  const auto direction = segment.End - segment.Start;
  const auto poly1 = common::Polygon{
    {segment.Start, segment.End, segment.Start + direction.Rotate(alpha)},
  };
  const auto poly2 = common::Polygon{
    {segment.Start, segment.End, segment.Start + direction.Rotate(-alpha)}, 
  };

  return {poly1, poly2};
}

double GetSpeed(const RouteData& route_data, const double wave_height) {
  const auto& ship_performance_info = route_data.ShipPerformanceInfo;
  if (
      !ship_performance_info.DangerHeight.has_value() ||
      !ship_performance_info.EnginePower.has_value() ||
      !ship_performance_info.Displacement.has_value() ||
      !ship_performance_info.Length.has_value() ||
      !ship_performance_info.Fullness.has_value() ||
      !ship_performance_info.ShipDraft.has_value()
  ) {
    return ship_performance_info.Speed.value();
  }
  auto r = common::CalculateVelocityRatio(
    ship_performance_info.EnginePower.value(),
    ship_performance_info.Displacement.value(),
    ship_performance_info.Length.value(),
    ship_performance_info.Fullness.value(),
    wave_height
  );

  return r * ship_performance_info.Speed.value();
}

std::vector<entities::diagnostic::DiagnosticHazardPoint> FilterDiagnosticPoints(
  const std::vector<entities::diagnostic::DiagnosticHazardPoint>& points, double distance_between_points) {
  std::vector<entities::diagnostic::DiagnosticHazardPoint> result;

  for(const auto& point : points) {
    if (result.size() == 0 
        || common::GetHaversineDistance(result.back().GetLocation(), point.GetLocation()) >= distance_between_points) {
      result.push_back(point);
    }
  }
  return result;
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
  double cur_speed = route_data_.ShipPerformanceInfo.Speed.value();

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

    if (i > 0) {
      const auto& previous_point = route_points[i - 1];
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

    if (nearest_forecast->GetWaveHeight() > route_data_.ShipPerformanceInfo.DangerHeight.value()) {
      result.push_back(entities::diagnostic::MakeHighWavesHazardPoint(
        route_point.nearest_forecast->point,
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
  std::vector<common::Polygon> polygons;
  std::vector<RoutePointWithForecast> start_points;

  for(size_t i = 0; i < route.size(); ++i) {
    const auto& point = route[i];
    const auto segment_id = point.route_point.segment_id;
    if (i == 0 || route[i - 1].route_point.segment_id != segment_id) {
      const auto& segment = route_data_.Route->GetSegments()[segment_id];
      const auto alpha = common::CalculateSteeringAngle(point.speed);
      const auto [poly1, poly2] = MakeDepthCheckPolygon(segment.segment, alpha);
      polygons.push_back(poly1);
      polygons.push_back(poly2);
      start_points.push_back(point);
      start_points.push_back(point);
    }
  }

  const auto hazard_triangle_points = db_client_->SelectHazardDepthPoints(polygons, route_data_.ShipPerformanceInfo.DangerHeight.value());
  std::vector<entities::diagnostic::DiagnosticHazardPoint> result;
  for(size_t i = 0; i < hazard_triangle_points.size(); ++i) {
    const auto& start_point = start_points[i];
    const auto& triangel_points = hazard_triangle_points[i];
    for(const  auto& depth_point : triangel_points) {
      const time_t expected_time = start_point.expected_time + common::GetHaversineDistance(start_point.route_point.point, depth_point.Point) / start_point.speed;
      result.push_back(entities::diagnostic::MakeDepthHazardPoint(depth_point.Point, check_time, expected_time, depth_point.Depth));
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

  auto hazard_points = FilterDiagnosticPoints(diagnostic_depth, 10000);
  const auto filtered_diagnostic_forecast = FilterDiagnosticPoints(diagnostic_forecast, 10000);
  hazard_points.insert(hazard_points.end(), filtered_diagnostic_forecast.begin(), filtered_diagnostic_forecast.end());

  return entities::diagnostic::RouteValidateDiagnostic{
    .result = entities::diagnostic::RouteValidateDiagnostic::DiagnosticResultType::kWarning,
    .hazard_points = hazard_points
  };
}

}  // namespace marine_navi::cases
