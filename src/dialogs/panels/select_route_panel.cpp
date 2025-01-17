#include "select_route_panel.h"

#include <memory>

#include <ocpn_plugin.h>

namespace marine_navi::dialogs::panels {

namespace {

wxArrayString GetRouteNames() {
  wxArrayString result;
  wxArrayString routeGUIDArray = GetRouteGUIDArray();
  for (const auto& routeGUID : routeGUIDArray) {
    std::unique_ptr<PlugIn_Route> route = GetRoute_Plugin(routeGUID);
    result.Add(route->m_NameString);
  }
  return result;
}

std::vector<PlugIn_Waypoint> GetWaypoints(wxString route_name) {
  std::vector<PlugIn_Waypoint> result;
  wxArrayString routeGUIDArray = GetRouteGUIDArray();
  for (const auto& routeGUID : routeGUIDArray) {
    std::unique_ptr<PlugIn_Route> route = GetRoute_Plugin(routeGUID);
    if (route->m_NameString == route_name) {
      auto* node = route->pWaypointList->GetFirst();
      while (node) {
        result.push_back(*(node->GetData()));
        node = node->GetNext();
      }
    }
  }
  return result;
}

void RefreshRouteList(wxListBox* boxList) {
  boxList->Clear();
  boxList->InsertItems(GetRouteNames(), 0);
}

} // namespace

SelectRoutePanel::SelectRoutePanel(wxWindow* parent, const std::string& title): wxPanel(parent) {
    wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);
    main_sizer->Add(new wxStaticText(this, wxID_ANY, title), 0, wxALL, 5);
    list_box_ = new wxListBox(this, wxID_ANY);
    b_refresh_list_ = new wxButton(this, wxID_ANY, _("Refresh"));
    main_sizer->Add(list_box_, 1, wxALL | wxEXPAND, 5);
    main_sizer->Add(b_refresh_list_, 0, wxALL, 5);

    SetSizerAndFit(main_sizer);
    Centre(wxBOTH);
    BindRefreshList();
}

std::shared_ptr<entities::Route> SelectRoutePanel::GetRoute() const {
    int selection = list_box_->GetSelection();
    if (selection == wxNOT_FOUND) {
      return nullptr;
    }

    return std::make_shared<entities::Route>(
      entities::Route(GetWaypoints(list_box_->GetString(selection))));
}

void SelectRoutePanel::OnRefresh(wxCommandEvent& event) {
    RefreshRouteList(list_box_);
}

void SelectRoutePanel::BindRefreshList() {
    b_refresh_list_->Bind(wxEVT_BUTTON, &SelectRoutePanel::OnRefresh, this);
}
void SelectRoutePanel::UnbindRefreshList() {
    b_refresh_list_->Unbind(wxEVT_BUTTON, &SelectRoutePanel::OnRefresh, this);
}

} // namespace marine_navi::dialogs::panels
