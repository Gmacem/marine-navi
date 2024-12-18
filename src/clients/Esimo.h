#pragma once

#include "common/csv_parser.h"
#include "common/HttpClient.h"

#include "ForecastsProvider.h"

#include "ocpn_plugin.h"

#include <wx/curl/http.h>

#include <optional>
#include <string>
#include <vector>

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

  Utils::HttpClient curl_;

  std::string url;
  std::string responseBody_;

  std::vector<entities::ForecastRecord> records_;
};

}  // namespace marine_navi::clients
