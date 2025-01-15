#pragma once

#include <memory>

#include "clients/db_client.h"
#include "entities/route.h"
#include "entities/ship.h"

namespace marine_navi::cases {

struct BestRouteInput {
  std::shared_ptr<entities::Route> route;
  std::shared_ptr<entities::Route> bounds;
  entities::ShipPerformanceInfo ship_performance_info;
  time_t depart_time;

  enum class ScoreType {
    kTime,
    kFuel
  } score_type;
};

struct BestRouteResult {
  std::vector<common::Point> points;

  time_t arrival_time;
};

class BestRouteMaker{
public:
    BestRouteMaker(std::shared_ptr<clients::DbClient> db_client);

    BestRouteResult MakeBestRoute(const BestRouteInput& input);

private:
    std::shared_ptr<clients::DbClient> db_client_;

};

} // namespace marine_navi::cases
