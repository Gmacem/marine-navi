#pragma once


#include <mutex>
#include <optional>

#include <wx/wx.h>

#include <ocpn_plugin.h>

#include <clients/DbClient.h>
#include <common/Utils.h>
#include <entities/DepthGrid.h>
#include <entities/Diagnostic.h>
#include <entities/Route.h>

namespace marine_navi::cases {

struct PathData {
  std::shared_ptr<entities::Route> Route;
  double Speed;
  std::optional<double> ShipDraft;
  std::optional<double> MaxWaveHeight;
  time_t DepartTime;
  std::optional<std::string> PathToDepthFile;
};

class CheckPathCase {
  using Point = Utils::Point;

public:
  CheckPathCase(std::shared_ptr<clients::DbClient> dbClient);
  void SetPathData(const PathData& pathData);
  const PathData& GetPathData();
  void SetShow(bool show);
  bool IsShow();

  bool CheckLandIntersection(const Point& p1, const Point& p2) const;
  bool CheckDepth(const entities::DepthGrid& grid, const Point& p, double draft) const;
  void CrossDetect();

  std::optional<entities::Diagnostic> GetDiagnostic();

private:
  std::optional<entities::Diagnostic> DoCrossDetect() const;

private:
  std::mutex mutex_;
  PathData pathData_;
  bool show_;
  std::shared_ptr<clients::DbClient> dbClient_;
  std::optional<entities::Diagnostic> diagnostic_;
};

}  // namespace marine_navi::cases
