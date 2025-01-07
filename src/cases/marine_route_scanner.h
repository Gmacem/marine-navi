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

namespace marine_navi::cases {

struct RouteData {
  std::shared_ptr<entities::Route> Route;
  std::optional<double> DangerHeight;
  std::optional<double> EnginePower;
  std::optional<double> Displacement;
  std::optional<double> Length;
  std::optional<double> Fullness;
  std::optional<double> Speed;
  std::optional<double> ShipDraft;
  time_t DepartTime;
  std::optional<std::string> PathToDepthFile;
};

class MarineRouteScanner {
  using Point = common::Point;

public:
  MarineRouteScanner(std::shared_ptr<clients::DbClient> dbClient);
  void SetPathData(const RouteData& pathData);
  const RouteData& GetPathData();
  void SetShow(bool show);
  bool IsShow();

  bool CheckLandIntersection(const Point& p1, const Point& p2) const;
  bool CheckDepth(const entities::DepthGrid& grid, const Point& p, double draft) const;
  void CrossDetect();

  std::optional<entities::diagnostic::RouteValidateDiagnostic> GetDiagnostic();

private:
  std::vector<entities::diagnostic::DiagnosticHazardPoint> GetForecastDiagnostic() const;
  std::vector<entities::diagnostic::DiagnosticHazardPoint> GetDepthDiagnostic() const;
  entities::diagnostic::RouteValidateDiagnostic DoCrossDetect() const;

private:
  std::mutex mutex_;
  RouteData route_data_;
  bool show_;
  std::shared_ptr<clients::DbClient> db_client_;
  std::optional<entities::diagnostic::RouteValidateDiagnostic> diagnostic_;
};

}  // namespace marine_navi::cases
