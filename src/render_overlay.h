#pragma once

#include <pidc.h>
#include <wx/wx.h>

#include "ocpn_plugin.h"

#include "cases/marine_route_scanner.h"
#include "dependencies.h"

namespace marine_navi {

class RenderOverlay {
public:
  RenderOverlay(Dependencies& deps);

  bool Render(piDC& dc, PlugIn_ViewPort* vp);

private:
  void RenderCheckPath(piDC& dc, PlugIn_ViewPort* vp, const cases::RouteData& pathData);

private:
  std::shared_ptr<cases::MarineRouteScanner> checkPathCase_;

  bool checkPathCalculated_;
  std::optional<wxPoint2DDouble> checkPathResult_;

  std::vector<PlugIn_Waypoint_Ex> diagnostic_waypoints_;
};

}  // namespace marine_navi
