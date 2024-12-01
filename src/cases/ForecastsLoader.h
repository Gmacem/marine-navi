#pragma once

#include "clients/DbClient.h"
#include "clients/ForecastsProvider.h"

#include <future>
#include <thread>

namespace MarineNavi::cases {

class ForecastsLoader {
public:
  ForecastsLoader(std::shared_ptr<DbClient> dbClient);

  void Load();  // TODO: load from different regions

  ~ForecastsLoader();

private:
  std::shared_ptr<DbClient> dbClient_;
  std::shared_ptr<IForecastsProvider> forecastsProvider_;

  std::thread loadThread_;
  std::future<void> future_;
};
}  // namespace MarineNavi::cases
