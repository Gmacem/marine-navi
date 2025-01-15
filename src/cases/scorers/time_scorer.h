#pragma once

#include "cases/helpers/forecast_accessor.h"
#include "cases/helpers/route_helpers.h"
#include "cases/scorers/iscore.h"
#include "clients/db_client.h"

namespace marine_navi::cases::scorers {

class TimeScorer : public IScorer {
public:
  TimeScorer(const entities::ShipPerformanceInfo& info,
             const std::vector<common::Point>& route_points,
             std::shared_ptr<clients::DbClient> db_client, time_t min_time);

  int64_t GetScore(int start_id, int end_id, time_t depart_time) override;
  time_t GetArrivalTime(int start_id, int end_id, time_t depart_time) override;

private:
  const entities::ShipPerformanceInfo ship_performance_info_;
  const std::vector<common::Point> route_points_;
  std::shared_ptr<clients::DbClient> db_client_;
  const time_t min_time_;
  const helpers::ForecastAccessor forecast_accessor_;
};

}  // namespace marine_navi::cases::scorers
