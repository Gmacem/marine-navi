#include "best_route_maker.h"

#include <queue>

#include "cases/scorers/iscore.h"
#include "cases/scorers/time_scorer.h"
#include "entities/find_route_grid.h"

namespace marine_navi::cases {

namespace {

entities::FindRouteGrid MakeFindRouteGrid(const BestRouteInput& input) {
  static constexpr double kStep = 0.1;  // size of grid cell in radians
  const auto bound_points = input.bounds->GetPoints();
  std::vector<common::Point> points;

  for (const auto& bound_point : bound_points) {
    points.push_back(bound_point.point);
  }
  return entities::FindRouteGrid{common::Polygon{points}, kStep};
}

BestRouteResult MakeBestRouteWithScorer(
    const entities::FindRouteGrid& find_route_grid, int start_point_id,
    int end_point_id, std::shared_ptr<scorers::IScorer> scorer) {

  const auto& points = find_route_grid.GetPoints();

  std::vector<int64_t> dp(points.size(), scorers::IScorer::kMaxScore);
  std::vector<time_t> expected_time(points.size(), 0);
  std::vector<int64_t> prev(points.size(), -1);

  using ValueType = std::pair<int64_t, int>; // score, point_id

  std::priority_queue<ValueType,
                      std::vector<ValueType>,
                      std::greater<ValueType> > order;
  dp[start_point_id] = 0;
  order.push({0, start_point_id});

  while (!order.empty()) {
    const auto [score, point_id] = order.top();
    const auto depart_time = expected_time[point_id];
    order.pop();
    if (dp[point_id] != score) {
      continue;
    }

    if (point_id == end_point_id) {
      break;
    }
    for (const auto& adjency_point_id : find_route_grid.GetAdjencyPointIds(point_id)) {
      const auto adjency_point_score = score + scorer->GetScore(point_id, adjency_point_id, depart_time);
      if (dp[adjency_point_id] < adjency_point_score) {
        dp[adjency_point_id] = adjency_point_score;
        prev[adjency_point_id] = point_id;
        expected_time[adjency_point_id] = scorer->GetArrivalTime(point_id, adjency_point_id, depart_time);
        order.push({adjency_point_score, adjency_point_id});
      }
    }
  }

  std::vector<common::Point> result;
  int point_id = end_point_id;
  while (point_id != -1) {
    result.push_back(points.at(point_id));
    point_id = prev.at(point_id);
  }
  std::reverse(result.begin(), result.end());
  return BestRouteResult{
      .points = result,
      .arrival_time = expected_time[end_point_id]
  };
}

}  // namespace

BestRouteMaker::BestRouteMaker(std::shared_ptr<clients::DbClient> db_client)
    : db_client_(db_client) {}

BestRouteResult BestRouteMaker::MakeBestRoute(const BestRouteInput& input) {
  const auto find_route_grid = MakeFindRouteGrid(input);

  if (input.route->GetSegments().size() != 1) {
    std::runtime_error("route must have only one segment");
  }
  const auto& route_segment = input.route->GetSegments()[0];

  int start_point_id =
      find_route_grid.GetClosestPointId(route_segment.segment.Start);
  int end_point_id =
      find_route_grid.GetClosestPointId(route_segment.segment.End);

  std::shared_ptr<scorers::IScorer> scorer;
  switch (input.score_type) {
    case BestRouteInput::ScoreType::kTime:
      scorer = std::make_shared<scorers::TimeScorer>(db_client_);
      break;
    case BestRouteInput::ScoreType::kFuel:
      break;
      //input.scorer = std::make_shared<scorers::FuelScorer>(db_client_);
    default:
      std::runtime_error("unknown score type");
  }
  return MakeBestRouteWithScorer(find_route_grid, start_point_id, end_point_id, scorer);
}

}  // namespace marine_navi::cases
