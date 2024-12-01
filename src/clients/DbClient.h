#pragma once

#include "ForecastsProvider.h"

#include "entities/WeatherForecast.h"
#include "common/Utils.h"

#include <SQLiteCpp/Database.h>
#include <SQLiteCpp/Transaction.h>

#include <memory>
#include <string>

namespace MarineNavi {

class DbClient {
public:
  DbClient(std::shared_ptr<SQLite::Database> db) : db_(db) {}

  int64_t InsertForecast(ForecastsSource source);
  int64_t InsertForecastRecord(std::string startedAt, std::string endAt,
                               double lat, double lon,
                               std::optional<double> waveHeight,
                               std::optional<double> swellHeight,
                               int64_t forecastId);  // TODO: batch upsert

  void InsertForecastRecords(const std::vector<ForecastRecord>& records,
                             int64_t forecastId);

  std::vector<
      std::tuple<int, int, std::optional<double>, std::optional<double>>>
  SelectNearestForecasts(
      const std::vector<std::pair<int, MarineNavi::Utils::Point>>&
          points_with_id,
      const std::string& date);

  Utils::Point GetForecastLocation(int forecastId);

private:
  int64_t InsertQuery(std::string query);

private:
  std::shared_ptr<SQLite::Database> db_;
};

std::shared_ptr<SQLite::Database> CreateDatabase(std::string dbName);

}  // namespace MarineNavi
