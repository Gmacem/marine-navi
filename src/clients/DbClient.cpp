#include "DbClient.h"

#include "ocpn_plugin.h"

#include <iomanip>
#include <string>

#include <wx/filename.h>
#include <wx/log.h>

namespace marine_navi::clients {

namespace {
constexpr int BUFFER_SIZE = 1024;

using BaseArgVar = clients::query_builder::BaseArgVar;
using SingleArgVar = clients::query_builder::SingleArgVar;
using ComposedArgVar = clients::query_builder::ComposedArgVar;

}  // namespace

int64_t DbClient::InsertForecast(marine_navi::entities::ForecastsSource source) {
  const std::string kQueryName = "kInsertForecast";
  const auto& query_template = query_storage_->GetTemplate(kQueryName);

  const auto query = query_template.MakeQuery(query_builder::ComposeArguments(source));

  return InsertQuery(query);
}

void DbClient::InsertForecastRecordBatch(const std::vector<marine_navi::entities::ForecastRecord>& records,
                                         int64_t forecastId) {
  static constexpr int BATCH_SIZE = 512;
  const std::string kQueryName = "kInserForecastRecordBatchQuery";
  const auto& query_template = query_storage_->GetTemplate(kQueryName);

  SQLite::Transaction trans(*db_);
  for (size_t i = 0; i < records.size(); i += BATCH_SIZE) {
    size_t bound = std::min(i + BATCH_SIZE, records.size());
    std::vector<ComposedArgVar> outer;
    for (size_t j = i; j < bound; ++j) {
      std::vector<SingleArgVar> inner{
        BaseArgVar{records[j].StartedAt},
        BaseArgVar{records[j].Date},
        BaseArgVar{records[j].Lat},
        BaseArgVar{records[j].Lon},
        records[j].WaveHeight,
        records[j].SwellHeight,
        BaseArgVar{forecastId}
      };
      outer.emplace_back(std::move(inner));
    }
    db_->exec(query_template.MakeQuery(outer));
    wxLogInfo(_T("Load progress: %lu/%lu"), bound, records.size());
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

std::vector<std::tuple<int, std::optional<double>, std::optional<double>>>
DbClient::SelectNearestForecasts(
    const std::vector<std::pair<int, marine_navi::Utils::Point>>& points_with_id,
    const std::string& date) {

  const std::string kQueryName = "kSelectClosestForecasts";
  const auto& query_template = query_storage_->GetTemplate(kQueryName);

  std::vector<std::vector<SingleArgVar> > points(points_with_id.size());
  for (size_t i = 0; i < points_with_id.size(); ++i) {
    const auto& [id, point] = points_with_id[i];
    points[i] = std::vector<SingleArgVar>{
      BaseArgVar{id}, BaseArgVar{point.X()}, BaseArgVar{point.Y()}
    };
  }

  const auto query = query_template.MakeQuery(query_builder::ComposeArguments(points, date));
  
  SQLite::Statement st(*db_, query);

  std::vector<std::tuple<int, std::optional<double>, std::optional<double>>> result;

  while (st.executeStep()) {
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
    result.emplace_back(id, wave_height, swell_height);
  }
  return result;
}

std::shared_ptr<SQLite::Database> CreateDatabase(std::string db_name, std::shared_ptr<query_builder::SqlQueryStorage> query_storage) {
  const std::string kQueryName = "kCreateTables";
  const auto& query_template = query_storage->GetTemplate(kQueryName);

  try {
    wxString sep = wxFileName::GetPathSeparator();
    wxString dbPath =
        *GetpPrivateApplicationDataLocation() + sep + "plugins" + sep + db_name;

    auto db = std::make_shared<SQLite::Database>(
        dbPath, SQLite::OPEN_CREATE | SQLite::OPEN_READWRITE);

    db->exec(query_template.MakeQuery({}));
    return db;
  } catch (std::exception& ex) {
    wxLogError(_T("Failed to create datbase with reason: %s"), ex.what());
    throw ex;
  } catch (...) {
    wxLogError(_T("Failed to create datbase with unknown reason"));
    throw;
  }
}

}  // namespace marine_navi::clients
