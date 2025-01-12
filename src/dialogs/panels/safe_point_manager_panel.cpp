#include "safe_point_manager_panel.h"

#include <ocpn_plugin.h>

namespace marine_navi::dialogs::panels {

namespace {
  wxArrayString GetWaypointNames() {
    wxArrayString result;
    wxArrayString waypointGUIDArray = GetWaypointGUIDArray();
    for (const auto& waypointGUID : waypointGUIDArray) {
      std::unique_ptr<PlugIn_Waypoint> waypoint = GetWaypoint_Plugin(waypointGUID);
      result.Add(waypoint->m_MarkName);
    }
    return result;
  }

  void RefreshWaypointList(wxListBox* boxList) {
    boxList->Clear();
    boxList->InsertItems(GetWaypointNames(), 0);
  }

  std::optional<PlugIn_Waypoint> GetWaypoint(wxString waypoint_name) {
    wxArrayString waypointGUIDArray = GetWaypointGUIDArray();
    for (const auto& waypointGUID : waypointGUIDArray) {
      std::unique_ptr<PlugIn_Waypoint> waypoint = GetWaypoint_Plugin(waypointGUID);
      if (waypoint->m_MarkName == waypoint_name) {
        return *waypoint;
      }
    }
    return std::nullopt;
  }
}

SafePointManagerPanel::SafePointManagerPanel(wxWindow* parent,
                                             const Dependencies& dependencies)
    : wxPanel(parent), safe_point_manager_(dependencies.safe_point_manager) {
  wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);

  waypoint_list_box_ = new wxListBox(this, wxID_ANY);
  b_refresh_waypoint_list_ = new wxButton(this, wxID_ANY, _("Browse..."));
  b_add_safe_point_ = new wxButton(this, wxID_ANY, _("Add safe point"));

  main_sizer->Add(waypoint_list_box_, 1, wxALL | wxEXPAND, 5);
  main_sizer->Add(b_refresh_waypoint_list_, 0, wxALL, 5);
  main_sizer->Add(b_add_safe_point_, 0, wxALL, 5);

  SetSizerAndFit(main_sizer);
  Centre(wxBOTH);
  BindEvents();
}

void SafePointManagerPanel::BindEvents() {
  b_refresh_waypoint_list_->Bind(wxEVT_BUTTON, &SafePointManagerPanel::OnRefreshSafePointList, this);
  b_add_safe_point_->Bind(wxEVT_BUTTON, &SafePointManagerPanel::OnAddSafePoint, this);
}

void SafePointManagerPanel::UnbindEvents() {
  b_refresh_waypoint_list_->Unbind(wxEVT_BUTTON, &SafePointManagerPanel::OnRefreshSafePointList, this);
  b_add_safe_point_->Unbind(wxEVT_BUTTON, &SafePointManagerPanel::OnAddSafePoint, this);
}

void SafePointManagerPanel::OnRefreshSafePointList(wxCommandEvent&) {
  RefreshWaypointList(waypoint_list_box_);
}

void SafePointManagerPanel::OnAddSafePoint(wxCommandEvent&) {
  int selection = waypoint_list_box_->GetSelection();
  if (selection == wxNOT_FOUND) {
    wxMessageBox("Please choose a waypoint.", "Error", wxOK | wxICON_ERROR);
    return;
  }
  const auto waypoint_name = waypoint_list_box_->GetString(selection);
  const auto waypoint = GetWaypoint(waypoint_name);
  if (!waypoint.has_value())  {
    wxMessageBox("Waypoint not found. Please choose other waypoint.", "Error", wxOK | wxICON_ERROR);
    return;
  }
  safe_point_manager_->Load(entities::SafePoint{
    common::Point{waypoint.value().m_lat, waypoint.value().m_lon},
    waypoint_name.ToStdString()
  });
}

}  // namespace marine_navi::dialogs::panels
