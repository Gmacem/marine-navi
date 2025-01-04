#pragma once


#include <mutex>
#include <optional>

#include <wx/wx.h>

#include <ocpn_plugin.h>

#include <clients/db_client.h>
#include <common/utils.h>
#include <entities/depth_grid.h>
#include <entities/diagnostic.h>
#include <entities/route.h>

namespace marine_navi::cases {

struct RouteData {
  std::shared_ptr<entities::Route> Route;
  double Speed;
  std::optional<double> ShipDraft;
  std::optional<double> MaxWaveHeight;
  time_t DepartTime;
  std::optional<std::string> PathToDepthFile;
};

class MarineRouteScanner {
  using Point = Utils::Point;

public:
  MarineRouteScanner(std::shared_ptr<clients::DbClient> dbClient);
  void SetPathData(const RouteData& pathData);
  const RouteData& GetPathData();
  void SetShow(bool show);
  bool IsShow();

  bool CheckLandIntersection(const Point& p1, const Point& p2) const;
  bool CheckDepth(const entities::DepthGrid& grid, const Point& p, double draft) const;
  void CrossDetect();

  std::optional<entities::RouteValidateDiagnostic> GetDiagnostic();

private:
  std::optional<entities::RouteValidateDiagnostic> DoCrossDetect() const;

private:
  std::mutex mutex_;
  RouteData pathData_;
  bool show_;
  std::shared_ptr<clients::DbClient> dbClient_;
  std::optional<entities::RouteValidateDiagnostic> diagnostic_;
};

}  // namespace marine_navi::cases
