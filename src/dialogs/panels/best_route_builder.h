#pragma once

#include <string>
#include <optional>

#include <wx/wx.h>

#include "cases/best_route_maker.h"
#include "dialogs/panels/depart_time_input.h"
#include "dialogs/panels/select_route_panel.h"
#include "dialogs/panels/ship_info_panel.h"
#include "render_overlay.h"

#include "dependencies.h"

namespace marine_navi::dialogs::panels {

class BestRouteBuilderPanel : public wxPanel {
public:
  BestRouteBuilderPanel(wxWindow* parent, const Dependencies& dependencies);
  ~BestRouteBuilderPanel() { UnbindEvents(); }

private:
  void OnMakeBestRoute(wxCommandEvent& event);
  void BindEvents();
  void UnbindEvents();

private:
    ShipInfoPanel* ship_info_panel_;
    SelectRoutePanel* select_zone_panel_;
    SelectRoutePanel* select_route_panel_;
    DepartTimeInput* depart_time_input_;
    wxButton* b_make_best_route_;

    std::shared_ptr<cases::BestRouteMaker> best_route_maker_;

    std::shared_ptr<RenderOverlay> render_overlay_;
    wxWindow* canvas_window_;
};

} // namespace marine_navi::dialogs:panels
