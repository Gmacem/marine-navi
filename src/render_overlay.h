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

  void RenderBestPath(PlugIn_Route_Ex* route_ex);

private:
  void RenderCheckPath(piDC& dc, PlugIn_ViewPort* vp, const cases::RouteScannerInput& pathData);

private:
  std::shared_ptr<cases::MarineRouteScanner> checkPathCase_;

  std::optional<wxPoint2DDouble> checkPathResult_;
  wxWindow* canvas_window_;
};

}  // namespace marine_navi
