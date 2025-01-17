#include "forecasts_loader.h"

#include <wx/log.h>

#include "clients/esimo.h"
#include "common/utils.h"

namespace marine_navi::cases {
namespace {

using namespace std::chrono_literals;
const std::string kBlackSea = "RU_Hydrometcentre_69";

}  // namespace

ForecastsLoader::ForecastsLoader(std::shared_ptr<clients::DbClient> db_client)
    : db_client_(db_client),
      forecasts_provider_(std::make_shared<clients::EsimoProvider>(kBlackSea)) {}

void ForecastsLoader::Load() {
  if (load_thread_.joinable()) {
    if (future_.wait_for(0ms) != std::future_status::ready) {
      throw std::runtime_error("esimo forecast loading not finished yet");
    }
    load_thread_.join();
  }

  std::packaged_task<void()> task([this] {
    wxLogInfo(_T("Start esimo loading task"));
    forecasts_provider_->LoadForecasts();
    auto forecast = forecasts_provider_->GetForecast();
    auto records = forecasts_provider_->GetRecords();

    try {
      int64_t forecastId = db_client_->InsertForecast(forecast.Source);
      db_client_->InsertForecastRecordBatch(records, forecastId);
    } catch (SQLite::Exception& ex) {
      wxLogError(_T("Failed to load forecasts with reason: %s"), ex.what());
      throw ex;
    }
    wxLogInfo(_T("Finish esimo loading task\n"));
  });
  future_ = task.get_future();
  load_thread_ = std::thread(std::move(task));
}

ForecastsLoader::~ForecastsLoader() {
  if (load_thread_.joinable()) {
    load_thread_.join();  // TODO maybe should kill
  }
}

}  // namespace marine_navi::cases
