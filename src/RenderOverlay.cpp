#include "RenderOverlay.h"

namespace marine_navi {

RenderOverlay::RenderOverlay(Dependencies& deps)
    : checkPathCase_(deps.CheckPathCase) {}

bool RenderOverlay::Render(piDC& dc, PlugIn_ViewPort* vp) {
  if (checkPathCase_->IsShow()) {
    auto pathData = checkPathCase_->GetPathData();
    RenderCheckPath(dc, vp, pathData);
    return true;
  }
  return false;
}

void RenderOverlay::RenderCheckPath(piDC& dc, PlugIn_ViewPort* vp,
                                    const cases::PathData& pathData) {
  dc.SetPen(*wxBLACK);                // reset pen
  dc.SetBrush(*wxTRANSPARENT_BRUSH);  // reset brush
  dc.SetPen(wxPen(wxColor(0, 0, 0)));
  wxPen pen = dc.GetPen();
  pen.SetWidth(5);
  dc.SetPen(pen);

  auto cross = checkPathCase_->GetDiagnostic();
  if (cross.has_value()) {
    wxPoint2DDouble crossCenter;
    GetDoubleCanvasPixLL(vp, &crossCenter, cross->Location.Y(),
                         cross->Location.X());
    dc.DrawCircle(round(crossCenter.m_x), round(crossCenter.m_y), 10);
  }
}

}  // namespace marine_navi
