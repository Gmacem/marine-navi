#pragma once

#include <optional>
#include <string>
#include <vector>

#include <wx/curl/http.h>

#include "ocpn_plugin.h"

#include "clients/forecasts_provider.h"
#include "common/csv_parser.h"
#include "common/http_client.h"

namespace marine_navi::clients {

class EsimoProvider : public clients::IForecastsProvider {
public:
  EsimoProvider(const std::string& resourceId,
                std::optional<std::string> filter = std::nullopt);

  void LoadForecasts() override;
  entities::Forecast GetForecast() override;
  std::vector<entities::ForecastRecord> GetRecords() override;
  ~EsimoProvider() override = default;

private:
  wxString SaveData(const std::string& data);

private:
  std::string resourceId_;

  common::HttpClient curl_;

  std::string url;
  std::string responseBody_;

  std::vector<entities::ForecastRecord> records_;
};

}  // namespace marine_navi::clients
