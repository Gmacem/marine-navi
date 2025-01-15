#pragma once


#include <mutex>
#include <optional>

#include <wx/wx.h>

#include <ocpn_plugin.h>

#include "clients/db_client.h"
#include "common/geom.h"
#include "common/utils.h"
#include "entities/depth_grid.h"
#include "entities/diagnostic/diagnostic.h"
#include "entities/diagnostic/diagnostic_hazard_point.h"
#include "entities/route.h"
#include "entities/ship.h"

namespace marine_navi::cases {

struct RouteScannerInput {
  std::shared_ptr<entities::Route> Route;
  entities::ShipPerformanceInfo ShipPerformanceInfo;

  time_t DepartTime;
};

class MarineRouteScanner {
  using Point = common::Point;

public:
  MarineRouteScanner(std::shared_ptr<clients::DbClient> dbClient);
  void SetPathData(const RouteScannerInput& pathData);
  const RouteScannerInput& GetPathData();
  void SetShow(bool show);
  bool IsShow();
  void CrossDetect();

  std::optional<entities::diagnostic::RouteValidateDiagnostic> GetDiagnostic();

private:
  struct RoutePointWithForecast {
    entities::RoutePoint route_point;
    std::optional<entities::ForecastPoint> closest_forecast;

    double speed;
    time_t expected_time;
  };
  std::vector<RoutePointWithForecast> GetRouteInfo() const;

  std::vector<entities::diagnostic::DiagnosticHazardPoint> GetForecastDiagnostic(
    const std::vector<RoutePointWithForecast>& route,
    const time_t check_time) const;
  std::vector<entities::diagnostic::DiagnosticHazardPoint> GetDepthDiagnostic(
    const std::vector<RoutePointWithForecast>& route,
    const time_t check_time) const;
  entities::diagnostic::RouteValidateDiagnostic DoCrossDetect() const;

private:
  std::mutex mutex_;
  RouteScannerInput route_data_;
  bool show_;
  std::shared_ptr<clients::DbClient> db_client_;
  std::optional<entities::diagnostic::RouteValidateDiagnostic> diagnostic_;
};

}  // namespace marine_navi::cases
