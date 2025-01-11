#include "render_overlay.h"

namespace marine_navi {

RenderOverlay::RenderOverlay(Dependencies& deps)
    : checkPathCase_(deps.marine_route_scanner) {}

bool RenderOverlay::Render(piDC& dc, PlugIn_ViewPort* vp) {
  if (checkPathCase_->IsShow()) {
    auto pathData = checkPathCase_->GetPathData();
    RenderCheckPath(dc, vp, pathData);
    return true;
  }
  return false;
}

void RenderOverlay::RenderCheckPath(piDC& dc, PlugIn_ViewPort* vp,
                                    const cases::RouteData& pathData) {
  dc.SetPen(*wxBLACK);                // reset pen
  dc.SetBrush(*wxTRANSPARENT_BRUSH);  // reset brush
  dc.SetPen(wxPen(wxColor(0, 0, 0)));
  wxPen pen = dc.GetPen();
  pen.SetWidth(5);
  dc.SetPen(pen);

  auto cross = checkPathCase_->GetDiagnostic();

  const auto guids = GetWaypointGUIDArray();
  for(const auto& guid : guids) {
    wxLogInfo(_T("guid waypoint '%s'"), guid.ToStdString().c_str());
  }

  for(auto& waypoint : diagnostic_waypoints_) {
    DeleteSingleWaypoint(waypoint.m_GUID);
  }

  if (cross.has_value() && cross->result == entities::diagnostic::RouteValidateDiagnostic::DiagnosticResultType::kWarning) {
    wxPoint2DDouble crossCenter;
    for(size_t i = 0; i < cross->hazard_points.size(); ++i) {
      const auto& hazard_point = cross->hazard_points[i];
      const auto location = hazard_point.GetLocation();
      const wxString guid = "hazard_points_" + std::to_string(i);
      PlugIn_Waypoint_Ex waypoint{location.Lat, location.Lon, wxEmptyString, wxEmptyString, guid};
      waypoint.m_MarkDescription = hazard_point.GetMessage();
      waypoint.RangeRingColor = wxColor(255, 0, 0);
      AddSingleWaypointEx(&waypoint, false);
      // GetDoubleCanvasPixLL(vp, &crossCenter, location.Lat, location.Lon);
      // dc.DrawCircle(round(crossCenter.m_x), round(crossCenter.m_y), 10);
    }
  }
}

}  // namespace marine_navi
