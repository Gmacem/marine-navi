#include "db_client.h"

#include <sqlite3.h>
#include <wx/filename.h>
#include <wx/log.h>

#include "ocpn_plugin.h"

#include <iomanip>
#include <string>

namespace marine_navi::clients {

namespace {

using BaseArgVar = clients::query_builder::BaseArgVar;
using SingleArgVar = clients::query_builder::SingleArgVar;
using ComposedArgVar = clients::query_builder::ComposedArgVar;
using Point = common::Point;

template <
    typename RecordType,
    typename FormatFunc,
    // SFINAE: Enable only if FormatFunc can be invoked with const RecordType& and returns std::vector<SingleArgVar>
    typename = std::enable_if_t<
        std::is_invocable_r_v<std::vector<SingleArgVar>, FormatFunc, const RecordType&>
    >
>
std::vector<std::string> MakeBatchQuery(
    const std::vector<RecordType>& records,
    const query_builder::SqlTemplate& query_template,
    FormatFunc format_func
) {
  static constexpr int BATCH_SIZE = 1024;
  std::vector<std::string> result;
  result.reserve((records.size() + BATCH_SIZE - 1) / BATCH_SIZE);
  for (size_t i = 0; i < records.size(); i += BATCH_SIZE) {
      size_t bound = std::min(i + BATCH_SIZE, records.size());
      std::vector<std::vector<SingleArgVar>> outer;
      outer.reserve(bound - i);

      for (size_t j = i; j < bound; ++j) {
          outer.emplace_back(format_func(records[j]));
      }
      const auto query = query_template.MakeQuery(query_builder::ComposeArguments(outer));
      result.push_back(query);
  }

  return result;
}

}  // namespace

int64_t DbClient::InsertForecast(
    marine_navi::entities::ForecastsSource source) {
  const std::string kQueryName = "kInsertForecast";
  const auto& query_template = query_storage_->GetTemplate(kQueryName);

  const auto query =
      query_template.MakeQuery(query_builder::ComposeArguments(source));

  return InsertQuery(query);
}

void DbClient::InsertForecastRecordBatch(
    const std::vector<marine_navi::entities::ForecastRecord>& records,
    int64_t forecastId) {
  const std::string kQueryName = "kInserForecastRecordBatchQuery";
  const auto& query_template = query_storage_->GetTemplate(kQueryName);

  auto format_func = [&](const marine_navi::entities::ForecastRecord& record) -> std::vector<SingleArgVar> {
      return std::vector<SingleArgVar>{
          BaseArgVar{record.StartedAt},
          BaseArgVar{record.Date},
          SingleArgVar{record.WaveHeight},
          SingleArgVar{record.SwellHeight},
          BaseArgVar{forecastId},
          BaseArgVar{common::Point{record.Lat, record.Lon}}
      };
  };
  std::vector<std::string> queries = MakeBatchQuery(records, query_template, format_func);

  SQLite::Transaction trans(*db_);
  for(const auto& query : queries) {
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

std::vector<std::tuple<entities::ForecastPoint, double, int>>
DbClient::SelectClosestForecasts(
    const std::vector<common::Point>& route_points,
    const double max_distance_rad,
    const time_t& min_date) {
  const std::string kQueryName = "kSelectClosestForecasts";
  const auto& query_template = query_storage_->GetTemplate(kQueryName);

  std::vector<std::vector<SingleArgVar> > points_with_id;
  for(size_t i = 0; i < route_points.size(); ++i) {
    points_with_id.emplace_back(std::vector<SingleArgVar>{
        BaseArgVar{static_cast<int64_t>(i)},
        BaseArgVar{route_points[i]},
    });
  }
  const std::string date = common::ToString(min_date);

  const auto query =
      query_template.MakeQuery(query_builder::ComposeArguments(points_with_id, max_distance_rad, date));

  SQLite::Statement st(*db_, query);
  std::vector<std::tuple<entities::ForecastPoint, double, int>> result;

  while (st.executeStep()) {
    entities::ForecastPoint forecast_point;
    forecast_point.point = common::Point::FromWktString(st.getColumn(0).getText());
    forecast_point.started_at = common::ParseDate(st.getColumn(1).getText());
    forecast_point.end_at = common::ParseDate(st.getColumn(2).getText());

    auto wave_column = st.getColumn(3);
    auto swell_columm = st.getColumn(4);
    if (!wave_column.isNull()) {
      forecast_point.wave_height = wave_column.getDouble();
    }
    if (!swell_columm.isNull()) {
      forecast_point.swell_height = swell_columm.getDouble();
    }
    result.emplace_back(forecast_point, st.getColumn(5).getDouble(), st.getColumn(6).getInt());
  }
  return result;
}

Point DbClient::SelectForecastLocation(int forecast_id) {
  const std::string kQueryName = "kSelectForecastLocation";
  const auto& query_template = query_storage_->GetTemplate(kQueryName);

  const auto query =
      query_template.MakeQuery(query_builder::ComposeArguments(forecast_id));

  SQLite::Statement st(*db_, query);

  if (st.executeStep()) {
    return Point{st.getColumn(0).getDouble(),
                 st.getColumn(1).getDouble()};
  }
  wxLogError(_T("Failed to select forecast location '%d'"), forecast_id);
  return Point{};
}

std::shared_ptr<SQLite::Database> CreateDatabase(
    std::string db_name,
    std::shared_ptr<query_builder::SqlQueryStorage> query_storage) {
  const std::string kQueryName = "kCreateTables";
  const char* kSpatialExtension = "mod_spatialite.so";

  const auto& query_template = query_storage->GetTemplate(kQueryName);

  try {
    wxString sep = wxFileName::GetPathSeparator();
    wxString dbPath =
        *GetpPrivateApplicationDataLocation() + sep + "plugins" + sep + db_name;

    auto db = std::make_shared<SQLite::Database>(
        dbPath, SQLite::OPEN_CREATE | SQLite::OPEN_READWRITE);

    sqlite3_enable_load_extension(db->getHandle(), 1);
    if (sqlite3_load_extension(db->getHandle(), kSpatialExtension, nullptr, nullptr) != SQLITE_OK) {
        throw std::runtime_error("error loading extension");
    }

    db->exec(query_template.MakeQuery({}));
    return db;
  } catch (SQLite::Exception& ex) {
    wxLogError(_T("Failed to create database with reason: %s"), ex.what());
    throw ex;
  } catch (std::exception& ex) {
    wxLogError(_T("Failed to create database with reason: %s"), ex.what());
    throw ex;
  } catch (...) {
    wxLogError(_T("Failed to create database with unknown reason"));
    throw;
  }
}

void DbClient::InsertDepthPointBatch(const std::vector<entities::DepthPoint>& records) {
  const std::string kQueryName = "kInsertDepths";
  const auto& query_template = query_storage_->GetTemplate(kQueryName);
  auto format_func = [](const entities::DepthPoint& record) {
    return std::vector<SingleArgVar>{
        BaseArgVar{record.Depth},
        BaseArgVar{record.Point}
    };
  };
  const auto queries = MakeBatchQuery(records, query_template, format_func);

  // SQLite::Transaction trans(*db_);
  for(size_t i = 0; i < queries.size(); ++i) {
    const auto& query = queries[i];
    db_->exec(query);
    wxLogInfo(_T("depth load progress %zu/%zu"), (i+1), queries.size());
  }
  // trans.commit();
}

std::vector<std::vector<entities::DepthPoint> > DbClient::SelectHazardDepthPoints(std::vector<common::Polygon> triangles, double height) {
  const std::string kQueryName = "kSelectHazardDepthPoints";
  const auto& query_template = query_storage_->GetTemplate(kQueryName);

  std::vector<std::vector<SingleArgVar> > triangles_with_id(triangles.size());
  for(size_t i = 0; i < triangles.size(); ++i) {
    triangles_with_id[i] = std::vector<SingleArgVar>{
      BaseArgVar{static_cast<int64_t>(i)},
      BaseArgVar{triangles[i]},
      BaseArgVar{height}
    };
  }

  const auto query = query_template.MakeQuery(query_builder::ComposeArguments(triangles_with_id));
  SQLite::Statement st(*db_, query);
  std::vector<std::vector<entities::DepthPoint> > result(triangles.size());
  while (st.executeStep()) {
    const double depth = st.getColumn(0).getDouble();
    const Point point = Point::FromWktString(st.getColumn(1).getText());
    const int triangle_id = st.getColumn(2).getInt();
    result[triangle_id].push_back({entities::DepthPoint{point, depth}});
  }
  return result;
}

void DbClient::InsertSafePoints(const std::vector<entities::SafePoint>& save_points) {
  const std::string kQueryName = "kInsertSafePoints";
  const auto& query_template = query_storage_->GetTemplate(kQueryName);

  std::vector<std::vector<SingleArgVar> > save_points_for_insert;
  for(size_t i = 0; i < save_points.size(); ++i) {
    const auto& save_point = save_points[i];
    save_points_for_insert.emplace_back(std::vector<SingleArgVar>{
        BaseArgVar{save_point.Name},
        BaseArgVar{save_point.Point},
    });
  }

  const auto query = query_template.MakeQuery(query_builder::ComposeArguments(save_points_for_insert));
  db_->exec(query);
}

std::vector<entities::SafePoint> DbClient::SelectSafePoints() {
  const std::string kQueryName = "kInsertSafePoints";
  const auto& query_template = query_storage_->GetTemplate(kQueryName);

  const auto query = query_template.MakeQuery({});
  SQLite::Statement st(*db_, query);
  std::vector<entities::SafePoint> result;
  while (st.executeStep()) {
    const std::string name = st.getColumn(0).getText();
    const Point point = Point::FromWktString(st.getColumn(1).getText());
    result.push_back({entities::SafePoint{point, name}});
  }
  return result;
}

}  // namespace marine_navi::clients
