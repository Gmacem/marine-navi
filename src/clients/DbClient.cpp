#include "DbClient.h"

#include "ocpn_plugin.h"
#include <wx/filename.h>

#include <iomanip>
#include <string>

namespace MarineNavi {

namespace {
constexpr int BUFFER_SIZE = 1024;
char const* kInsertForecastQuery =
    "INSERT INTO forecasts (source) "
    "VALUES (%d) "
    "RETURNING id";

char const* kInsertForecastRecordQuery =
    "INSERT INTO forecast_records ("
    "started_at, end_at, lat, lon, wave_height, swell_height, forecast_id) "
    "VALUES ('%s', '%s', %lf, %lf, %s, %s, %d) "
    "RETURNING id";

char const* kInserForecastRecordBatchQuery =
    "INSERT INTO forecast_records ("
    "started_at, end_at, lat, lon, wave_height, swell_height, forecast_id) "
    "VALUES ";

char const* kSelectClosestForecasts =
    "WITH path(id, X, Y) AS ("
    "  VALUES %s"
    "), fwd as ("
    "  SELECT forecast_records.id as fid, path.id as pid,"
    "    (forecast_records.lon - path.X) * (forecast_records.lon - path.X) "
    "      + (forecast_records.lat - path.Y) * (forecast_records.lat - path.Y) "
    "      as dst, forecast_records.wave_height, forecast_records.swell_height "
    "  from forecast_records "
    "  CROSS JOIN path "
    "  WHERE '%s' < forecast_records.end_at) "
    "SELECT mfwd.dst, fwd.dst, fwd.fid, fwd.pid, fwd.wave_height, "
    "fwd.swell_height FROM fwd "
    "INNER JOIN ( "
    "  select fid, min(dst) as dst FROM fwd GROUP BY pid) mfwd "
    "on fwd.dst = mfwd.dst and fwd.fid = mfwd.fid";

char const* kSelectForecastLocation =
    "SELECT lat, lon FROM forecast_records "
    "WHERE id = %d";

}  // namespace

int64_t DbClient::InsertForecast(ForecastsSource source) {
  char buff[BUFFER_SIZE];
  sprintf(buff, kInsertForecastQuery, source);
  return InsertQuery(buff);
}

int64_t DbClient::InsertForecastRecord(std::string startedAt, std::string endAt,
                                       double lat, double lon,
                                       std::optional<double> waveHeight,
                                       std::optional<double> swellHeight,
                                       int64_t forecastId) {
  char buff[BUFFER_SIZE];

  std::string waveHeightS = "NULL";
  std::string swellHeightS = "NULL";

  if (waveHeight.has_value()) {
    waveHeightS = std::to_string(waveHeight.value());
  }
  if (swellHeight.has_value()) {
    swellHeightS = std::to_string(swellHeight.value());
  }

  sprintf(buff, kInsertForecastRecordQuery, startedAt.data(), endAt.data(), lat,
          lon, waveHeightS.data(), swellHeightS.data(), forecastId);
  return InsertQuery(buff);
}

void DbClient::InsertForecastRecords(const std::vector<ForecastRecord>& records,
                                     int64_t forecastId) {
  static constexpr int BATCH_SIZE = 512;

  SQLite::Transaction trans(*db_);
  int cur = 0;
  for (size_t i = 0; i < records.size(); i += BATCH_SIZE) {
    fprintf(stderr, "Load progress: %d/%lu\n", cur++,
            (records.size() + BATCH_SIZE - 1) / BATCH_SIZE);
    std::stringstream ss;
    ss << kInserForecastRecordBatchQuery;
    size_t bound = std::min(i + BATCH_SIZE, records.size());
    for (size_t j = i; j < bound; ++j) {
      const auto& record = records[j];

      std::string waveHeightS = "NULL";
      std::string swellHeightS = "NULL";

      if (record.WaveHeight.has_value()) {
        waveHeightS = std::to_string(record.WaveHeight.value());
      }
      if (record.SwellHeight.has_value()) {
        swellHeightS = std::to_string(record.SwellHeight.value());
      }

      ss << std::fixed << std::setprecision(3) << "('" << record.StartedAt
         << "', '" << record.Date << "', " << record.Lat << ", " << record.Lon
         << ", " << waveHeightS << ", " << swellHeightS << ", " << forecastId
         << ")";
      if (j + 1 < bound) {
        ss << ", ";
      }
    }
    std::string query = ss.str();
    db_->exec(query);
  }
  trans.commit();
}

int64_t DbClient::InsertQuery(std::string query) {
  try {
    auto result = db_->execAndGet(query);
    return result.getInt();
  } catch (std::exception& ex) {
    fprintf(stderr, "Failed to exec query %s with error: %s", query.data(),
            ex.what());
    throw ex;
  }
}

std::shared_ptr<SQLite::Database> CreateDatabase(std::string dbName) {
  try {
    wxString sep = wxFileName::GetPathSeparator();
    wxString dbPath =
        *GetpPrivateApplicationDataLocation() + sep + "plugins" + sep + dbName;

    auto db = std::make_shared<SQLite::Database>(
        dbPath, SQLite::OPEN_CREATE | SQLite::OPEN_READWRITE);

    SQLite::Transaction trans(*db);
    db->exec(
        "CREATE TABLE IF NOT EXISTS forecasts ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "created_at TEXT DEFAULT CURRENT_TIMESTAMP NOT NULL, "
        "source INTEGER NOT NULL)");
    db->exec(
        "CREATE TABLE IF NOT EXISTS forecast_records ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "created_at TEXT DEFAULT CURRENT_TIMESTAMP NOT NULL, "
        "started_at TEXT NOT NULL, "
        "end_at TEXT NOT NULL, "
        "lat REAL NOT NULL, "
        "lon REAL NOT NULL, "
        "wave_height REAL, "
        "swell_height REAL, "
        "forecast_id INTEGER NOT NULL, "
        "FOREIGN KEY(forecast_id) REFERENCES forecasts(id))");
    db->exec(
        "CREATE UNIQUE INDEX IF NOT EXISTS forecast_records_idx ON "
        "forecast_records ("
        "started_at, end_at, lat, lon, forecast_id)");
    trans.commit();
    return db;
  } catch (std::exception& ex) {
    fprintf(stderr, "Failed to create datbase with reason: %s\n", ex.what());
    throw ex;
  } catch (...) {
    fprintf(stderr, "Failed to create datbase with unknown reason");
    throw;
  }
}

std::vector<std::tuple<int, int, std::optional<double>, std::optional<double>>>
DbClient::SelectNearestForecasts(
    const std::vector<std::pair<int, MarineNavi::Utils::Point>>& points_with_id,
    const std::string& date) {
  std::stringstream ss;
  for (size_t i = 0; i < points_with_id.size(); ++i) {
    const auto& [id, point] = points_with_id[i];
    ss << "(" << id << ", " << point.X() << ", " << point.Y() << ")";
    if (i + 1 < points_with_id.size()) {
      ss << ", ";
    }
  }
  std::string query_vals = ss.str();

  auto query = MarineNavi::Utils::StringFormat(kSelectClosestForecasts,
                                               ss.str().data(), date.data());
  fprintf(stderr, "Nearest point query: %s\n", query.data());
  SQLite::Statement st(*db_, query);

  std::vector<
      std::tuple<int, int, std::optional<double>, std::optional<double>>>
      result;

  while (st.executeStep()) {
    int forecastId = st.getColumn(2).getInt();
    int id = st.getColumn(3).getInt();
    auto wave_column = st.getColumn(4);
    auto swell_columm = st.getColumn(5);

    std::optional<double> wave_height;
    std::optional<double> swell_height;
    if (!wave_column.isNull()) {
      wave_height = wave_column.getDouble();
    }
    if (!swell_columm.isNull()) {
      swell_height = swell_columm.getDouble();
    }
    result.emplace_back(forecastId, id, wave_height, swell_height);
  }
  return result;
}

Utils::Point DbClient::GetForecastLocation(int forecastId) {
  auto query =
      MarineNavi::Utils::StringFormat(kSelectForecastLocation, forecastId);
  fprintf(stderr, "Forecast location query: %s\n", query.data());
  SQLite::Statement st(*db_, query);

  Utils::Point location;
  if (st.executeStep()) {
    location =
        Utils::Point{st.getColumn(0).getDouble(), st.getColumn(1).getDouble()};
  }
  return location;
}

}  // namespace MarineNavi
