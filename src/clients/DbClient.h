#pragma once

#include <memory>
#include <string>

#include <SQLiteCpp/Database.h>
#include <SQLiteCpp/Transaction.h>

#include "entities/WeatherForecast.h"
#include "common/Utils.h"

#include "ForecastsProvider.h"
#include "clients/query_builder/SqlQueryStorage.h"

namespace marine_navi::clients {

using SqlQueryStorage = query_builder::SqlQueryStorage;

class DbClient {
public:
  DbClient(std::shared_ptr<SQLite::Database> db) : db_(db) {}

  int64_t InsertForecast(entities::ForecastsSource source);
  void InsertForecastRecordBatch(const std::vector<entities::ForecastRecord>& records,
                                 int64_t forecastId);

  std::vector<std::tuple<int, std::optional<double>, std::optional<double>>>
  SelectNearestForecasts(
      const std::vector<std::pair<int, marine_navi::Utils::Point>>&
          points_with_id,
      const std::string& date);

private:
  int64_t InsertQuery(std::string query);

private:
  std::shared_ptr<SqlQueryStorage> query_storage_;
  std::shared_ptr<SQLite::Database> db_;
};

std::shared_ptr<SQLite::Database> CreateDatabase(std::string db_name, std::shared_ptr<SqlQueryStorage> query_storage);

}  // namespace marine_navi::clients
