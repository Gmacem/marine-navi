#include "esimo.h"

#include <chrono>
#include <filesystem>
#include <fstream>

#include <wx/log.h>

#include "common/utils.h"

namespace marine_navi::clients {
namespace {
namespace fs = std::filesystem;

using Forecast = marine_navi::entities::Forecast;
using ForecastRecord = marine_navi::entities::ForecastRecord;
using ForecastsSource = marine_navi::entities::ForecastsSource;

std::string BuildUrl() { return "http://esimo.ru/dataview/getresourceexport"; }

std::string GetBody(std::string resourceId) {
  return "format=csv&resourceid=" + resourceId + "&filter=";
}

inline wxString GetForecastDirPath() {
  wxString sep = wxFileName::GetPathSeparator();
  return *GetpPrivateApplicationDataLocation() + sep + "plugins" + sep +
         "forecasts";
}

int PushRow(void* params, int column_cnt, const char** columnValues) {
  if (column_cnt < 12) {
    wxLogWarning(
        _T("Esimo send broken csv with column count < 12, column count: %d"),
        column_cnt);
    return 1;
  }

  auto* records = static_cast<std::vector<ForecastRecord>*>(params);
  if (columnValues[0] == std::string("Дата и время: начало(минимум)")) {
    return 0;
  }
  try {
    records->push_back(ForecastRecord{
        .StartedAt = columnValues[0],
        .Date = columnValues[1],
        .Lat = std::stod(columnValues[2]),
        .Lon = std::stod(columnValues[3]),
        .WaveHeight = columnValues[6][0] == '\0'
                          ? std::nullopt
                          : std::optional<double>(std::stod(columnValues[6])),
        .SwellHeight = columnValues[10][0] == '\0'
                           ? std::nullopt
                           : std::optional<double>(std::stod(columnValues[10])),
    });
  } catch (const std::exception& ex) {
    wxLogError(_T("Failed to parse forecast record with reason: %s"),
               ex.what());
    wxLogError(
        _T("Values:\n\t2: %ld %s\n\t3: %ld %s\n\t6: %ld %s\n\t10: %ld %s"),
        strlen(columnValues[2]), columnValues[2], strlen(columnValues[3]),
        columnValues[3], strlen(columnValues[6]), columnValues[6],
        strlen(columnValues[10]), columnValues[10]);
  }
  return 0;
}

}  // namespace

EsimoProvider::EsimoProvider(const std::string& resourceId,
                             std::optional<std::string> filter)
    : resourceId_(resourceId), curl_(BuildUrl()) {
  curl_.AddHeader("Referer: http://esimo.ru/dataview/viewresource?resourceId=" +
                  resourceId);
}

void EsimoProvider::LoadForecasts() {
  std::string requestBody = GetBody(resourceId_);
  if (!curl_.Post(requestBody.data(), requestBody.size())) {
    fprintf(stderr, "Failed to load esimo forecast %s\n",
            curl_.GetErrorString().data());
    throw std::runtime_error("failed to load esimo forecast");
  }
  responseBody_ = curl_.GetResponseBody();
  auto path = SaveData(responseBody_);

  auto pFile = fopen(path.c_str(), "r");
  int r = csv_parse(pFile, PushRow, (void*)(&records_), 1);
  fclose(pFile);
  if (r != 0) {
    fprintf(stderr, "Failed to parse esimo forecast");
    return;
  }
}

Forecast EsimoProvider::GetForecast() {
  auto now = std::chrono::system_clock::now();
  auto startTime = common::ToString(std::chrono::system_clock::to_time_t(now));
  auto endTime = common::ToString(std::chrono::system_clock::to_time_t(now));

  for (const auto& record : records_) {
    startTime = std::min(startTime, record.StartedAt);
    endTime = std::min(endTime, record.Date);
  }
  return Forecast{
      .StartedAt = startTime,
      .FinishedAt = endTime,
      .Source = ForecastsSource::kEsimo,
  };
}

std::vector<ForecastRecord> EsimoProvider::GetRecords() { return records_; }

wxString EsimoProvider::SaveData(const std::string& data) {
  wxString sep = wxFileName::GetPathSeparator();

  // create directory for forecsts if not exist
  auto dirPath = GetForecastDirPath();
  if (!fs::exists(dirPath.data().AsChar())) {
    fs::create_directories(dirPath.data().AsChar());  // TODO not ignore result
  }

  int64_t ts = std::chrono::system_clock::now().time_since_epoch().count();
  wxString savePath = dirPath + sep + "esimo_" + std::to_string(ts) + ".csv";
  std::ofstream fout(savePath.data().AsChar());
  fout << data;
  return savePath;
}

}  // namespace marine_navi::clients
