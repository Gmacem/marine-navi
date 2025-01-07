#include "marine_route_scanner.h"

#include "common/marine_math.h"
#include "entities/depth_point.h"

namespace marine_navi::cases {

namespace {

void InsertDepth(const entities::DepthGrid& grid, std::shared_ptr<clients::DbClient> db_client) {
  const auto points = grid.GetAllPoints();
  db_client->InsertDepthPointBatch(points);
}

std::vector<entities::RoutePoint> GetRoutePoints(const std::shared_ptr<entities::Route>& route) {
  static constexpr double STEP_DIST_METERS = 100;
  std::vector<entities::RoutePoint> result;
  
  double total = route->GetDistance();

  for (size_t i = 0; i * STEP_DIST_METERS < total; ++i) {
    result.emplace_back(route->GetPointFromStart(i * STEP_DIST_METERS));
  }
  return result;
}

bool IsPointOnRoute(const common::Segment& segment, const common::Point point, double alpha) {
  const auto direction = segment.End - segment.Start;

  return common::IsInsideOfAngle(direction, point, direction.Rotate(alpha)) ||
         common::IsInsideOfAngle(direction, point, direction.Rotate(-alpha));
}

bool IsForecastDangerous(const entities::RouteSegment& route_segment,
                         const entities::ForecastPoint& forecast,
                         double speed,
                         time_t expected_time,
                         double danger_height) {
  double distance = common::GetHaversineDistance(route_segment.segment.Start, forecast.point);
  double t = distance / speed;
  expected_time += t;
  if (expected_time < forecast.started_at || expected_time > forecast.end_at) {
    return false;
  }
  double alpha = common::CalculateSteeringAngle(speed);
  if (!IsPointOnRoute(route_segment.segment, forecast.point, alpha)) {
    return false;
  }
  return danger_height < forecast.GetWaveHeight();
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

bool MarineRouteScanner::CheckDepth(const entities::DepthGrid& grid, const Point& p,
                                     double draft) const {
  auto depth = grid.GetDepth(p);

  if (depth.has_value()) {
    return -depth.value() > draft;  // TODO perhaps it needs to be taken into
                                    // account with some margin
  }

  return true;
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

std::vector<entities::diagnostic::DiagnosticHazardPoint> MarineRouteScanner::GetForecastDiagnostic() const {
  static constexpr double DANGEROUS_DISTANCE_METERS = 10000;
  if (!route_data_.DangerHeight.has_value()) {
    return {};
  }
  double danger_height = route_data_.DangerHeight.value();
  time_t depart_time = route_data_.DepartTime;

  const auto route_points = GetRoutePoints(route_data_.Route);
  std::vector<common::Point> points;
  std::transform(route_points.begin(), route_points.end(), std::back_inserter(points),
                 [](const entities::RoutePoint& route_point) { return route_point.point; });
  auto forecasts = db_client_->SelectNearestForecasts(points, DANGEROUS_DISTANCE_METERS, route_data_.DepartTime);

  std::vector<std::vector<std::pair<entities::ForecastPoint, double>>> grouped(route_points.size());

  for(const auto& forecast : forecasts) {
    int id = std::get<2>(forecast);
    grouped[id].emplace_back(std::get<0>(forecast), std::get<1>(forecast));
  }

  std::vector<entities::diagnostic::DiagnosticHazardPoint> result;
  time_t cur_time = depart_time;

  for(size_t i = 0; i < grouped.size(); ++i) {
    double wave_height = 0;
    for(const auto& forecast_with_distance : grouped[i]) {
      wave_height = std::max(wave_height, forecast_with_distance.first.GetWaveHeight());
    }
    double speed = GetSpeed(route_data_, wave_height);

    const size_t segment_id = route_points[i].segment_id;
    const auto& segment = route_data_.Route->GetSegments()[segment_id];
    wxLogInfo("check forecasts for segment %zu, speed %f, danger height %f, number of forecasts %zu",
      segment_id, speed, danger_height, grouped[i].size());

    for(const auto& forecast_with_distance : grouped[i]) {
      const auto& forecast = forecast_with_distance.first;
      if (IsForecastDangerous(segment, forecast, speed, cur_time, danger_height)) {
        result.push_back(entities::diagnostic::MakeHighWavesHazardPoint(
            forecast.point,
            speed,
            cur_time,
            forecast.GetWaveHeight()));
      }
    }

    if (i + 1 < route_points.size()) {
      cur_time += common::GetHaversineDistance(route_points[i].point, route_points[i+1].point) / speed;
    }
  }

  return result;
}

std::vector<entities::diagnostic::DiagnosticHazardPoint> MarineRouteScanner::GetDepthDiagnostic() const {
  const auto route = route_data_.Route;
  time_t check_time = common::GetCurrentTime();
  const auto route_points = GetRoutePoints(route_data_.Route);

  std::optional<entities::DepthGrid> grid;
  if (route_data_.PathToDepthFile.has_value() &&
      route_data_.ShipDraft.has_value()) {
    grid = entities::DepthGrid(route_data_.PathToDepthFile.value());
  } else {
    wxLogWarning("depth grid or ship draft is not provided");
    return {};
  }

  std::vector<entities::diagnostic::DiagnosticHazardPoint> result;

  for(const auto& route_point : route_points) {
    const auto depth_point = grid->GetNearestDepthPoint(route_point.point);
    if (depth_point.has_value() && -depth_point->Depth < route_data_.ShipDraft.value()) {
      result.push_back(entities::diagnostic::MakeDepthHazardPoint(
        depth_point->Point,
        check_time,
        check_time,
        depth_point->Depth
      )); // TODO: check time
    }
  }

  return result;
}

entities::diagnostic::RouteValidateDiagnostic MarineRouteScanner::DoCrossDetect() const {
  const auto diagnostic_forecast = GetForecastDiagnostic();
  const auto diagnostic_depth = GetDepthDiagnostic();

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
