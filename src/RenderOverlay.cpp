#include "RenderOverlay.h"

namespace marine_navi {

namespace {
using CheckPathCase = cases::CheckPathCase;
using PathData = cases::PathData;
} // namespace

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
                                    const PathData& pathData) {
  dc.SetPen(*wxBLACK);                // reset pen
  dc.SetBrush(*wxTRANSPARENT_BRUSH);  // reset brush
  dc.SetPen(wxPen(wxColor(0, 0, 0)));
  wxPen pen = dc.GetPen();
  pen.SetWidth(5);
  dc.SetPen(pen);

  wxPoint2DDouble ptD;
  GetDoubleCanvasPixLL(vp, &ptD, pathData.Start.Lat, pathData.Start.Lon);
  wxPoint pt1, pt2;
  pt1.x = round(wxRound(ptD.m_x));
  pt1.y = round(wxRound(ptD.m_y));
  GetDoubleCanvasPixLL(vp, &ptD, pathData.End.Lat, pathData.End.Lon);
  pt2.x = round(wxRound(ptD.m_x));
  pt2.y = round(wxRound(ptD.m_y));
  dc.DrawLine(pt1.x, pt1.y, pt2.x, pt2.y);

  auto cross = checkPathCase_->GetLastResult();
  if (cross.has_value()) {
    wxPoint2DDouble crossCenter;
    GetDoubleCanvasPixLL(vp, &crossCenter, cross->m_x, cross->m_y);
    dc.DrawCircle(round(crossCenter.m_x), round(crossCenter.m_y), 10);
  }
}

}  // namespace marine_navi
