#include "ForecastsLoader.h"

#include "clients/Esimo.h"
#include "common/Utils.h"

namespace MarineNavi::cases {
namespace {

using namespace std::chrono_literals;
const std::string kBlackSea = "RU_Hydrometcentre_69";

}  // namespace

ForecastsLoader::ForecastsLoader(std::shared_ptr<DbClient> dbClient)
    : dbClient_(dbClient),
      forecastsProvider_(std::make_shared<EsimoProvider>(kBlackSea)) {}

void ForecastsLoader::Load() {
  if (loadThread_.joinable()) {
    if (future_.wait_for(0ms) != std::future_status::ready) {
      throw std::runtime_error("esimo forecast loading not finished yet");
    }
    loadThread_.join();
  }

  std::packaged_task<void()> task([this] {
    fprintf(stderr, "Start esimo loading task\n");
    forecastsProvider_->LoadForecasts();
    auto forecast = forecastsProvider_->GetForecast();
    auto records = forecastsProvider_->GetRecords();

    int64_t forecastId = dbClient_->InsertForecast(forecast.Source);
    dbClient_->InsertForecastRecords(records, forecastId);
    fprintf(stderr, "Finish esimo loading task\n");
  });
  future_ = task.get_future();
  loadThread_ = std::thread(std::move(task));
}

ForecastsLoader::~ForecastsLoader() {
  if (loadThread_.joinable()) {
    loadThread_.join();  // TODO maybe should kill
  }
}

}  // namespace MarineNavi::cases
