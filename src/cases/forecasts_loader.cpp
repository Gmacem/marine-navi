#include "forecasts_loader.h"

#include <wx/log.h>

#include "clients/esimo.h"
#include "common/utils.h"

namespace marine_navi::cases {
namespace {

using namespace std::chrono_literals;
const std::string kBlackSea = "RU_Hydrometcentre_69";

}  // namespace

ForecastsLoader::ForecastsLoader(std::shared_ptr<clients::DbClient> dbClient)
    : db_client_(dbClient),
      forecastsProvider_(std::make_shared<clients::EsimoProvider>(kBlackSea)) {}

void ForecastsLoader::Load() {
  if (loadThread_.joinable()) {
    if (future_.wait_for(0ms) != std::future_status::ready) {
      throw std::runtime_error("esimo forecast loading not finished yet");
    }
    loadThread_.join();
  }

  std::packaged_task<void()> task([this] {
    wxLogInfo(_T("Start esimo loading task"));
    forecastsProvider_->LoadForecasts();
    auto forecast = forecastsProvider_->GetForecast();
    auto records = forecastsProvider_->GetRecords();

    int64_t forecastId = db_client_->InsertForecast(forecast.Source);
    db_client_->InsertForecastRecordBatch(records, forecastId);

    wxLogInfo(_T("Finish esimo loading task\n"));
  });
  future_ = task.get_future();
  loadThread_ = std::thread(std::move(task));
}

ForecastsLoader::~ForecastsLoader() {
  if (loadThread_.joinable()) {
    loadThread_.join();  // TODO maybe should kill
  }
}

}  // namespace marine_navi::cases
