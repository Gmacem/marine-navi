#include "best_route_builder.h"

#include "ocpn_plugin.h"

namespace marine_navi::dialogs::panels {

namespace {

PlugIn_Route_Ex* MakeRouteFromBestRouteResult(const marine_navi::cases::BestRouteResult& best_route_result) {
    PlugIn_Route_Ex* route = new PlugIn_Route_Ex;
    int id = 0;
    for (const auto& point : best_route_result.points) {
        auto* waypoint = new PlugIn_Waypoint_Ex(point.Lat, point.Lon, wxEmptyString, wxEmptyString, "best_rout_" + std::to_string(id++));
        route->pWaypointList->Append(waypoint);
    }

    return route;
}

} // namespace

BestRouteBuilderPanel::BestRouteBuilderPanel(wxWindow* parent, const Dependencies& dependencies):
        wxPanel(parent),
        best_route_maker_(dependencies.best_route_maker),
        render_overlay_(dependencies.render_overlay),
        canvas_window_(dependencies.ocpn_canvas_window) {
    ship_info_panel_ = new ShipInfoPanel(this);
    select_zone_panel_ = new SelectRoutePanel(this, "Select zone");
    select_route_panel_ = new SelectRoutePanel(this, "Select route");
    depart_time_input_ = new DepartTimeInput(this);
    b_make_best_route_ = new wxButton(this, wxID_ANY, _("Make best route"));

    wxBoxSizer* splitter = new wxBoxSizer(wxHORIZONTAL);

    wxBoxSizer* ext_ship_sizer = new wxBoxSizer(wxVERTICAL);
    ext_ship_sizer->Add(ship_info_panel_, 1, wxALL | wxEXPAND, 5);
    ext_ship_sizer->Add(depart_time_input_, 1, wxALL | wxEXPAND, 5);
    splitter->Add(ext_ship_sizer, 1, wxALL | wxEXPAND, 5);
    splitter->Add(select_zone_panel_, 1, wxALL | wxEXPAND, 5);
    splitter->Add(select_route_panel_, 1, wxALL | wxEXPAND, 5);

    wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);
    main_sizer->Add(splitter, 1, wxALL | wxEXPAND, 5);
    main_sizer->Add(b_make_best_route_, 0, wxALL, 5);

    SetSizerAndFit(main_sizer);
    Centre(wxBOTH);

    BindEvents();
}

void BestRouteBuilderPanel::OnMakeBestRoute(wxCommandEvent& event){
    auto route = select_route_panel_->GetRoute();
    if (route == nullptr) {
        wxMessageBox(_("No route selected"));
        return;
    }
    auto bounds = select_zone_panel_->GetRoute();
    if (bounds == nullptr) {
        wxMessageBox(_("No bounds selected"));
        return;
    }

    cases::BestRouteInput input {
        .route = route,
        .bounds = bounds,
        .ship_performance_info = ship_info_panel_->GetShipInfo(),
        .depart_time = depart_time_input_->GetTime(),
        .score_type = cases::BestRouteInput::ScoreType::kTime
    };
    const auto best_route = best_route_maker_->MakeBestRoute(input);
    auto* render_route = MakeRouteFromBestRouteResult(best_route);
    render_overlay_->RenderBestPath(render_route);
}

void BestRouteBuilderPanel::BindEvents() {
    b_make_best_route_->Bind(wxEVT_BUTTON, &BestRouteBuilderPanel::OnMakeBestRoute, this);
}
void BestRouteBuilderPanel::UnbindEvents() {
    b_make_best_route_->Unbind(wxEVT_BUTTON, &BestRouteBuilderPanel::OnMakeBestRoute, this);
}

} // namespace marine_navi::dialogs::panels
