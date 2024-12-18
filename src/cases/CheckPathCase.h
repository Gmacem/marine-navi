#pragma once

#include "clients/DbClient.h"
#include "common/Utils.h"
#include "entities/DepthGrid.h"

#include <ocpn_plugin.h>

#include <wx/wx.h>

#include <mutex>
#include <optional>

namespace marine_navi::cases {

struct PathData {
  Utils::Point Start;
  Utils::Point End;

  std::optional<double> ShipDraft;
  std::optional<std::string> PathToDepthFile;
  std::optional<double> MaxWaveHeight;
};

class CheckPathCase {
  using Point = Utils::Point;

public:
  CheckPathCase(std::shared_ptr<marine_navi::clients::DbClient> db_client);
  void SetPathData(const PathData& pathData);
  const PathData& GetPathData();
  void SetShow(bool show);
  bool IsShow();

  bool CheckLandIntersection(const Point& p1, const Point& p2) const;
  bool CheckDepth(const DepthGrid& grid, const Point& p, double draft) const;
  void CrossDetect();

  std::optional<wxPoint2DDouble> GetLastResult();

private:
  std::optional<wxPoint2DDouble> CrossDetectImpl() const;

private:
  std::mutex mutex_;
  PathData pathData_;
  bool show_;
  std::shared_ptr<marine_navi::clients::DbClient> db_client_;
  std::optional<wxPoint2DDouble> lastResult_;
};

}  // namespace marine_navi::cases
