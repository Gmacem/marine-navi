#pragma once

#include <memory>
#include <string>

#include <SQLiteCpp/Database.h>
#include <SQLiteCpp/Transaction.h>

#include "clients/query_builder/sql_query_storage.h"
#include "common/geom.h"
#include "common/utils.h"
#include "entities/depth_point.h"
#include "entities/forecast_point.h"
#include "entities/safe_point.h"
#include "entities/weather_forecast.h"

namespace marine_navi::clients {

using SqlQueryStorage = query_builder::SqlQueryStorage;

class DbClient {
public:
  DbClient(std::shared_ptr<SQLite::Database> db, std::shared_ptr<SqlQueryStorage> query_storage) 
    : db_(db), query_storage_(query_storage) {}

  int64_t InsertForecast(entities::ForecastsSource source);
  void InsertForecastRecordBatch(const std::vector<entities::ForecastRecord>& records,
                                 int64_t forecastId);

// @return forecast point, distance to point and point id
  std::vector<std::tuple<entities::ForecastPoint, double, int>>
  SelectClosestForecasts(
      const std::vector<common::Point>& route_points,
      const double max_distance_rad,
      const time_t& min_date);
  common::Point SelectForecastLocation(int forecast_id);

  void InsertDepthPointBatch(const std::vector<entities::DepthPoint>& depth_points);

  // @return A list of hazard points based on distance
  std::vector<std::vector<entities::DepthPoint> > SelectHazardDepthPoints(const std::vector<common::Point>& points, double height, double distance);

  // @return A list of hazard points for each triangle
  std::vector<std::vector<entities::DepthPoint> > SelectHazardDepthPointsInAngle(std::vector<common::Polygon> triagnles, double height);

  void InsertSafePoints(const std::vector<entities::SafePoint>& save_points);
  std::vector<entities::SafePoint> SelectSafePoints();

private:
  int64_t InsertQuery(std::string query);

private:
  std::shared_ptr<SQLite::Database> db_;
  std::shared_ptr<SqlQueryStorage> query_storage_;
};

std::shared_ptr<SQLite::Database> CreateDatabase(std::string db_name, std::shared_ptr<SqlQueryStorage> query_storage);

}  // namespace marine_navi::clients
