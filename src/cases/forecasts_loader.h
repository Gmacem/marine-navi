#pragma once

#include "clients/db_client.h"
#include "clients/forecasts_provider.h"

#include <future>
#include <thread>

namespace marine_navi::cases {

class ForecastsLoader {
public:
  ForecastsLoader(std::shared_ptr<clients::DbClient> db_client);

  void Load();  // TODO: load from different regions

  ~ForecastsLoader();

private:
  std::shared_ptr<clients::DbClient> db_client_;
  std::shared_ptr<clients::IForecastsProvider> forecasts_provider_;

  std::thread load_thread_;
  std::future<void> future_;
};
}  // namespace marine_navi::cases
