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
  if (cross.has_value() && cross->result == entities::diagnostic::RouteValidateDiagnostic::DiagnosticResultType::kWarning) {
    wxPoint2DDouble crossCenter;
    for(const auto& hazard_point : cross->hazard_points) {
      const auto location = hazard_point.GetLocation();
      GetDoubleCanvasPixLL(vp, &crossCenter, location.Lat, location.Lon);
      dc.DrawCircle(round(crossCenter.m_x), round(crossCenter.m_y), 10);
    }
  }
}

}  // namespace marine_navi
