#include "time_scorer.h"

#include "common/marine_math.h"

namespace marine_navi::cases::scorers {

namespace {
  constexpr double kMinRad = 0.1;

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

} // namespace

TimeScorer::TimeScorer(const entities::ShipPerformanceInfo& info,
                       const std::vector<common::Point>& route_points,
                       std::shared_ptr<clients::DbClient> db_client, time_t min_time):
  ship_performance_info_(info),
  route_points_(route_points),
  db_client_(db_client),
  min_time_(min_time),
  forecast_accessor_(db_client_->SelectClosestForecasts(route_points, kMinRad, min_time)) {}

int64_t TimeScorer::GetScore(int start_id, int end_id, time_t depart_time) {
  const auto start_point = route_points_.at(start_id);
  const auto end_point = route_points_.at(end_id);

  const auto forecast = forecast_accessor_.GetClosestForecast(start_id, depart_time);
  double wave_height = 0;
  if (forecast.has_value()) {
    wave_height = forecast->GetWaveHeight();
  }
  double speed = helpers::GetSpeed(ship_performance_info_, wave_height);
  
  {
    // TODO: make function
    std::vector<common::Polygon> polygons;
    const auto alpha = common::CalculateSteeringAngle(speed);
    const auto [poly1, poly2] = MakeDepthCheckPolygon({
      .Start = start_point,
      .End = end_point
    }, alpha);

    polygons.push_back(poly1);
    polygons.push_back(poly2);
    auto hazard_points = db_client_->SelectHazardDepthPoints(polygons, ship_performance_info_.DangerHeight.value());
    if (hazard_points[0].size() > 0) {
      return kMaxScore;
    }
  }

  return common::GetHaversineDistance(start_point, end_point) / speed;
}

time_t TimeScorer::GetArrivalTime(int start_id, int end_id, time_t depart_time) {
  const auto start_point = route_points_.at(start_id);
  const auto end_point = route_points_.at(end_id);

  const auto forecast = forecast_accessor_.GetClosestForecast(start_id, depart_time);
  double wave_height = 0;
  if (forecast.has_value()) {
    wave_height = forecast->GetWaveHeight();
  }
  double speed = helpers::GetSpeed(ship_performance_info_, wave_height);

  return depart_time + common::GetHaversineDistance(start_point, end_point) / speed;
}


} // namespace marine_navi::cases