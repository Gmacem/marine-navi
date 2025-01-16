#include "route_validate_panel.h"

#include <memory>

#include <wx/sizer.h>
#include <wx/valnum.h>

#include <ocpn_plugin.h>

#include <cases/depth_loader.h>
#include <cases/forecasts_loader.h>
#include <cases/marine_route_scanner.h>
#include <common/marine_math.h>
#include <dialogs/panels/helpers.h>

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

std::vector<PlugIn_Waypoint> GetRoute(wxString route_name) {
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

time_t GetTimeFromCtrls(wxDatePickerCtrl* date, wxTimePickerCtrl* time) {
  wxDateTime dateValue = date->GetValue();
  wxDateTime timeValue = time->GetValue();

  dateValue.SetHour(timeValue.GetHour());
  dateValue.SetMinute(timeValue.GetMinute());
  dateValue.SetSecond(timeValue.GetSecond());
  dateValue.SetMillisecond(timeValue.GetMillisecond());

  struct tm tmDateTime;

  tmDateTime.tm_year = dateValue.GetYear() - 1900;
  tmDateTime.tm_mon = dateValue.GetMonth();
  tmDateTime.tm_mday = dateValue.GetDay();
  tmDateTime.tm_hour = dateValue.GetHour();
  tmDateTime.tm_min = dateValue.GetMinute();
  tmDateTime.tm_sec = dateValue.GetSecond();
  tmDateTime.tm_isdst = -1;

  return timegm(&tmDateTime);
}

}  // namespace

RouteValidatePanel::RouteValidatePanel(wxWindow* parent,
                                       DiagnosticPanel* diagnostic_panel,
                                       const Dependencies& dependencies)
    : wxPanel(parent),
      diagnostic_panel_(diagnostic_panel),
      canvas_window_(dependencies.ocpn_canvas_window),
      marine_route_scanner_(dependencies.marine_route_scanner),
      depth_loader_(dependencies.depth_loader),
      forecasts_loader_(dependencies.forecasts_loader) {
  CreateControls();
  BindEvents();
}

void RouteValidatePanel::CreateControls() {
  wxBoxSizer* splitter = new wxBoxSizer(wxHORIZONTAL);

  wxPanel* firstPnl = new wxPanel(this, wxID_ANY);
  wxPanel* secondPnl = new wxPanel(this, wxID_ANY);
  wxBoxSizer* firstPanelSizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer* secondPanelSizer = new wxBoxSizer(wxVERTICAL);

  {    
    {
      ship_info_panel_ = new ShipInfoPanel(firstPnl);
      firstPanelSizer->Add(ship_info_panel_);
    }

    {
      wxBoxSizer* datetime = new wxBoxSizer(wxHORIZONTAL);
      wxBoxSizer* sDepartureDate = new wxBoxSizer(wxVERTICAL);
      sDepartureDate->Add(
          new wxStaticText(firstPnl, wxID_ANY, _("Departure date")));
      d_route_date_ = new wxDatePickerCtrl(firstPnl, wxID_ANY);
      sDepartureDate->Add(d_route_date_);

      wxBoxSizer* sDepartureTime = new wxBoxSizer(wxVERTICAL);
      sDepartureTime->Add(
          new wxStaticText(firstPnl, wxID_ANY, _("Departure time")));
      t_route_time_ = new wxTimePickerCtrl(firstPnl, wxID_ANY);
      sDepartureTime->Add(t_route_time_);

      datetime->Add(sDepartureDate, 1, wxALL, 5);
      datetime->Add(sDepartureTime, 1, wxALL, 5);
      firstPanelSizer->Add(datetime);
    }

    firstPnl->SetSizer(firstPanelSizer);
    splitter->Add(firstPnl, 1, wxALL | wxEXPAND, 5);
  }
  {
    b_browse_depth_file_button_ = new wxButton(secondPnl, wxID_ANY, _("Browse..."));
    b_refresh_route_list_ = new wxButton(secondPnl, wxID_ANY, _("Refresh"));
    route_list_box_ = new wxListBox(secondPnl, wxID_ANY);
    secondPanelSizer->Add(
        new wxStaticText(secondPnl, wxID_ANY, _("Route name")));
    secondPanelSizer->Add(route_list_box_, 1, wxALL | wxEXPAND, 5);
    secondPanelSizer->Add(b_refresh_route_list_, 0, wxALL, 5);

    c_depth_file_ = CreateBrowseFileTextCtrl(secondPnl, b_browse_depth_file_button_,
                                             secondPanelSizer, _("Depth file"));
    b_load_depth_ = new wxButton(secondPnl, wxID_ANY, _("Load depth"));

    secondPanelSizer->Add(b_load_depth_, 0, wxALL, 5);
    secondPnl->SetSizer(secondPanelSizer);
    splitter->Add(secondPnl, 1, wxALL | wxEXPAND, 5);
  }

  wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
  mainSizer->Add(splitter, 1, wxALL | wxEXPAND, 5);

  b_scan_route_ = new wxButton(this, wxID_ANY, _("Check path"));
  b_load_forecasts_ = new wxButton(this, wxID_ANY, _("Download forecasts"));
  mainSizer->Add(b_scan_route_, 0, wxALL | wxEXPAND, 5);
  mainSizer->Add(b_load_forecasts_, 0, wxALL | wxEXPAND, 5);

  SetSizerAndFit(mainSizer);
  Centre(wxBOTH);
}

void RouteValidatePanel::BindEvents() {
  b_scan_route_->Bind(wxEVT_BUTTON, &RouteValidatePanel::OnCheckPathClicked,
                    this);
  b_load_depth_->Bind(wxEVT_BUTTON, &RouteValidatePanel::OnLoadDepthClicked, this);
  b_load_forecasts_->Bind(wxEVT_BUTTON,
                        &RouteValidatePanel::OnLoadForecastsClicked, this);
  b_browse_depth_file_button_->Bind(
      wxEVT_BUTTON, &RouteValidatePanel::OnBrowseDepthClicked, this);
  b_refresh_route_list_->Bind(wxEVT_BUTTON,
                           &RouteValidatePanel::OnRefreshRouteList, this);
}

void RouteValidatePanel::UnbindEvents() {
  b_scan_route_->Unbind(wxEVT_BUTTON, &RouteValidatePanel::OnCheckPathClicked,
                      this);
  b_load_depth_->Unbind(wxEVT_BUTTON,
                        &RouteValidatePanel::OnLoadDepthClicked, this);
  b_load_forecasts_->Unbind(wxEVT_BUTTON,
                          &RouteValidatePanel::OnLoadForecastsClicked, this);
  b_browse_depth_file_button_->Unbind(
      wxEVT_BUTTON, &RouteValidatePanel::OnBrowseDepthClicked, this);
  b_refresh_route_list_->Unbind(wxEVT_BUTTON,
                             &RouteValidatePanel::OnRefreshRouteList, this);
}

void RouteValidatePanel::OnCheckPathClicked(wxCommandEvent&) {
  cases::RouteScannerInput route_data;

  if (int selection = route_list_box_->GetSelection(); selection == wxNOT_FOUND) {
    wxMessageBox("Please choose a route.", "Error", wxOK | wxICON_ERROR);
    return;
  } else {
    route_data.Route = std::make_shared<entities::Route>(
        entities::Route(GetRoute(route_list_box_->GetString(selection))));
    if (route_data.Route->GetSegments().size() < 1) {
      wxMessageBox("Invalid route.", "Error", wxOK | wxICON_ERROR);
      return;
    }
  }

  route_data.ShipPerformanceInfo = ship_info_panel_->GetShipInfo();
  if (!route_data.ShipPerformanceInfo.Speed.has_value()) {
    return;
  }
  route_data.DepartTime = GetTimeFromCtrls(d_route_date_, t_route_time_);

  marine_route_scanner_->SetPathData(route_data);
  marine_route_scanner_->SetShow(true);
  marine_route_scanner_->CrossDetect();
  const auto diagnostic = marine_route_scanner_->GetDiagnostic();
  if (diagnostic.has_value()) {
    fprintf(stderr, "Before call\n");
    diagnostic_panel_->UpdateDiagnostic(diagnostic.value());  // TODO remove reinterpret_cast
  }

  RequestRefresh(canvas_window_);
}


void RouteValidatePanel::OnLoadDepthClicked(wxCommandEvent&) {
  try {
    if (c_depth_file_->IsEmpty()) {
      throw std::runtime_error("depth file is not selected");
    }
    const auto filepath = c_depth_file_->GetValue().ToStdString();
    depth_loader_->Load(filepath);
  } catch (const std::exception& ex) {
    fprintf(stderr, "Failed to load depth: %s\n", ex.what());
  }
}

void RouteValidatePanel::OnLoadForecastsClicked(wxCommandEvent&) {
  try {
    forecasts_loader_->Load();
  } catch (const std::exception& ex) {
    fprintf(stderr, "Failed to load forecasts: %s\n", ex.what());
  }
}

void RouteValidatePanel::OnBrowseDepthClicked(wxCommandEvent& event) {
  wxFileDialog fileDialog(this, _("Select a file"), "", "",
                          "All files (*.*)|*.*",
                          wxFD_OPEN | wxFD_FILE_MUST_EXIST);
  if (fileDialog.ShowModal() == wxID_OK) {
    c_depth_file_->SetValue(fileDialog.GetPath());
  }
}

void RouteValidatePanel::OnRefreshRouteList(wxCommandEvent&) {
  RefreshRouteList(route_list_box_);
}

} // namespace marine_navi::dialogs:panels

