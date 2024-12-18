#include "MainNotebook.h"

#include "cases/CheckPathCase.h"
#include "cases/ForecastsLoader.h"

#include <ocpn_plugin.h>

#include <wx/button.h>
#include <wx/filedlg.h>
#include <wx/listbox.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/valnum.h>

#include <memory>

namespace MarineNavi::dialogs {

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

bool CheckIsEmpty(wxTextCtrl* txt, wxString message) {
  if (txt->IsEmpty()) {
    wxMessageBox(message, "Error", wxOK | wxICON_ERROR);
    return true;
  }
  return false;
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

  return mktime(&tmDateTime);
}

}  // namespace

// Base dialog class
MarineNaviDlgBase::MarineNaviDlgBase(wxWindow* parent, wxWindowID id,
                                     const wxString& title, const wxPoint& pos,
                                     const wxSize& size)
    : wxDialog(parent, id, title, pos, size,
               wxRESIZE_BORDER | wxDEFAULT_DIALOG_STYLE) {
  this->Connect(wxEVT_CLOSE_WINDOW,
                wxCloseEventHandler(MarineNaviDlgBase::OnClose));
}

void MarineNaviDlgBase::Register(std::function<void(void)> func,
                                 EventType event) {
  callbacks_[event].push_back(func);
}

MarineNaviDlgBase::~MarineNaviDlgBase() {
  this->Disconnect(wxEVT_CLOSE_WINDOW,
                   wxCloseEventHandler(MarineNaviDlgBase::OnClose));
}

void MarineNaviDlgBase::OnClose(wxCloseEvent& event) {
  for (auto& callback : callbacks_[EventType::kClose]) {
    callback();
  }
}
MarineNaviMainPanel::MarineNaviMainPanel(wxWindow* parent,
                                         MarineNaviMainDlg* dlg,
                                         const Dependencies& dependencies)
    : wxPanel(parent),
      dlg_(dlg),
      canvasWindow_(dependencies.OcpnCanvasWindow),
      checkPathCase_(dependencies.CheckPathCase),
      forecastsLoader_(dependencies.ForecastsLoader) {
  CreateControls();
  BindEvents();
}

void MarineNaviMainPanel::CreateControls() {
  wxBoxSizer* splitter = new wxBoxSizer(wxHORIZONTAL);

  wxPanel* firstPnl = new wxPanel(this, wxID_ANY);
  wxPanel* secondPnl = new wxPanel(this, wxID_ANY);
  wxBoxSizer* firstPanelSizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer* secondPanelSizer = new wxBoxSizer(wxVERTICAL);

  {
    bBrowseDepthFileButton_ = new wxButton(firstPnl, wxID_ANY, _("Browse..."));
    cSpeed_ = CreateLabeledTextCtrl(firstPanelSizer, _("Ship speed, kn"));
    cShipDraft_ = CreateLabeledTextCtrl(firstPanelSizer, _("Ship draft, m"));
    cMaxWave_ = CreateLabeledTextCtrl(firstPanelSizer, _("Max wave, m"));

    {
      wxBoxSizer* datetime = new wxBoxSizer(wxHORIZONTAL);
      wxBoxSizer* sDepartureDate = new wxBoxSizer(wxVERTICAL);
      sDepartureDate->Add(
          new wxStaticText(firstPnl, wxID_ANY, _("Departure date")));
      dRouteDate_ = new wxDatePickerCtrl(firstPnl, wxID_ANY);
      sDepartureDate->Add(dRouteDate_);

      wxBoxSizer* sDepartureTime = new wxBoxSizer(wxVERTICAL);
      sDepartureTime->Add(
          new wxStaticText(firstPnl, wxID_ANY, _("Departure time")));
      tRouteTime_ = new wxTimePickerCtrl(firstPnl, wxID_ANY);
      sDepartureTime->Add(tRouteTime_);

      datetime->Add(sDepartureDate, 1, wxALL, 5);
      datetime->Add(sDepartureTime, 1, wxALL, 5);
      firstPanelSizer->Add(datetime);
    }

    cDepthFile_ = CreateBrowseFileTextCtrl(bBrowseDepthFileButton_,
                                           firstPanelSizer, _("Depth file"));
    firstPnl->SetSizer(firstPanelSizer);
    splitter->Add(firstPnl, 1, wxALL | wxEXPAND, 5);
  }
  {
    bRefreshRouteList_ = new wxButton(secondPnl, wxID_ANY, _("Refresh"));
    routeListBox_ = new wxListBox(secondPnl, wxID_ANY);
    secondPanelSizer->Add(
        new wxStaticText(secondPnl, wxID_ANY, _("Route name")));
    secondPanelSizer->Add(routeListBox_, 1, wxALL | wxEXPAND, 5);
    secondPanelSizer->Add(bRefreshRouteList_, 0, wxALL, 5);
    secondPnl->SetSizer(secondPanelSizer);
    splitter->Add(secondPnl, 1, wxALL | wxEXPAND, 5);
  }

  wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
  mainSizer->Add(splitter, 1, wxALL | wxEXPAND, 5);

  bCheckPath_ = new wxButton(this, wxID_ANY, _("Check path"));
  bLoadForecasts_ = new wxButton(this, wxID_ANY, _("Download forecasts"));
  mainSizer->Add(bCheckPath_, 0, wxALL | wxEXPAND, 5);
  mainSizer->Add(bLoadForecasts_, 0, wxALL | wxEXPAND, 5);

  SetSizerAndFit(mainSizer);
  Centre(wxBOTH);
}

wxTextCtrl* MarineNaviMainPanel::CreateLabeledTextCtrl(wxSizer* sizer,
                                                       const wxString& label) {
  wxFloatingPointValidator<double> validator(2, nullptr, wxNUM_VAL_DEFAULT);
  validator.SetRange(-1e5, 1e5);
  sizer->Add(new wxStaticText(this, wxID_ANY, label));
  wxTextCtrl* textCtrl =
      new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
                     wxDefaultSize, 0, validator);
  sizer->Add(textCtrl, 0, wxALL, 5);
  return textCtrl;
}

wxTextCtrl* MarineNaviMainPanel::CreateBrowseFileTextCtrl(
    wxButton* browseButton, wxSizer* sizer, const wxString& label) {
  wxBoxSizer* fileSizer = new wxBoxSizer(wxHORIZONTAL);
  fileSizer->Add(new wxStaticText(this, wxID_ANY, label));
  wxTextCtrl* textCtrl = new wxTextCtrl(this, wxID_ANY, "");
  fileSizer->Add(textCtrl, 1, wxALL, 5);
  fileSizer->Add(browseButton, 0, wxALL, 5);
  sizer->Add(fileSizer, 1, wxALL | wxEXPAND, 5);
  return textCtrl;
}

void MarineNaviMainPanel::BindEvents() {
  bCheckPath_->Bind(wxEVT_BUTTON, &MarineNaviMainPanel::OnCheckPathClicked,
                    this);
  bLoadForecasts_->Bind(wxEVT_BUTTON,
                        &MarineNaviMainPanel::OnLoadForecastsClicked, this);
  bBrowseDepthFileButton_->Bind(
      wxEVT_BUTTON, &MarineNaviMainPanel::OnBrowseDepthClicked, this);
  bRefreshRouteList_->Bind(wxEVT_BUTTON,
                           &MarineNaviMainPanel::OnRefreshRouteList, this);
}

void MarineNaviMainPanel::UnbindEvents() {
  bCheckPath_->Unbind(wxEVT_BUTTON, &MarineNaviMainPanel::OnCheckPathClicked,
                      this);
  bLoadForecasts_->Unbind(wxEVT_BUTTON,
                          &MarineNaviMainPanel::OnLoadForecastsClicked, this);
  bBrowseDepthFileButton_->Unbind(
      wxEVT_BUTTON, &MarineNaviMainPanel::OnBrowseDepthClicked, this);
  bRefreshRouteList_->Unbind(wxEVT_BUTTON,
                             &MarineNaviMainPanel::OnRefreshRouteList, this);
}

void MarineNaviMainPanel::OnCheckPathClicked(wxCommandEvent&) {
  cases::PathData pathData;

  auto parseDouble = [](const wxString& str) -> std::optional<double> {
    double result;
    if (!str.ToDouble(&result)) {
      return std::nullopt;
    }
    return result;
  };

  if (int selection = routeListBox_->GetSelection(); selection == wxNOT_FOUND) {
    wxMessageBox("Please choose a route.", "Error", wxOK | wxICON_ERROR);
    return;
  } else {
    pathData.Route = std::make_shared<entities::Route>(
        entities::Route(GetRoute(routeListBox_->GetString(selection))));
    if (pathData.Route->GetRoute().size() < 2) {
      wxMessageBox("Invalid route.", "Error", wxOK | wxICON_ERROR);
      return;
    }
  }

  if (CheckIsEmpty(cSpeed_, _("Please specify speed"))) {
    return;
  }
  pathData.ShipDraft = parseDouble(cShipDraft_->GetValue());
  pathData.MaxWaveHeight = parseDouble(cMaxWave_->GetValue());
  pathData.PathToDepthFile =
      cDepthFile_->IsEmpty()
          ? std::nullopt
          : std::optional<std::string>(cDepthFile_->GetValue().mb_str());
  pathData.DepartTime = GetTimeFromCtrls(dRouteDate_, tRouteTime_);
  pathData.PathToDepthFile = cDepthFile_->GetValue();

  checkPathCase_->SetPathData(pathData);
  checkPathCase_->SetShow(true);
  checkPathCase_->CrossDetect();
  const auto diagnostic = checkPathCase_->GetDiagnostic();
  if (diagnostic.has_value()) {
    fprintf(stderr, "Before call\n");
    dlg_->UpdateDiagnostic(diagnostic.value());  // TODO remove reinterpret_cast
  }

  RequestRefresh(canvasWindow_);
}

void MarineNaviMainPanel::OnLoadForecastsClicked(wxCommandEvent&) {
  try {
    forecastsLoader_->Load();
  } catch (const std::exception& ex) {
    fprintf(stderr, "Failed to load forecasts: %s\n", ex.what());
  }
}

void MarineNaviMainPanel::OnBrowseDepthClicked(wxCommandEvent& event) {
  wxFileDialog fileDialog(this, _("Select a file"), "", "",
                          "All files (*.*)|*.*",
                          wxFD_OPEN | wxFD_FILE_MUST_EXIST);
  if (fileDialog.ShowModal() == wxID_OK) {
    cDepthFile_->SetValue(fileDialog.GetPath());
  }
}

void MarineNaviMainPanel::OnRefreshRouteList(wxCommandEvent&) {
  RefreshRouteList(routeListBox_);
}

DiagnosticPanel::DiagnosticPanel(wxWindow* parent) : wxPanel(parent) {
  cDiagnosticMessage_ =
      new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
                     wxDefaultSize, wxTE_READONLY | wxTE_MULTILINE);
  wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
  sizer->Add(cDiagnosticMessage_, 1, wxEXPAND | wxALL, 5);
  SetSizerAndFit(sizer);
}

void DiagnosticPanel::UpdateDiagnostic(const entities::Diagnostic& diagnostic) {
  fprintf(stderr, "After call %p %p\n", cDiagnosticMessage_, &diagnostic);
  auto message = entities::GetDiagnosticMessage(diagnostic);
  cDiagnosticMessage_->SetValue(message);
}

// Main dialog with tabs
MarineNaviMainDlg::MarineNaviMainDlg(wxWindow* parent, wxWindowID id,
                                     const wxString& title, const wxPoint& pos,
                                     const wxSize& size,
                                     const Dependencies& dependencies)
    : MarineNaviDlgBase(parent, id, title, pos, size) {
  wxNotebook* notebook = new wxNotebook(this, wxID_ANY);

  mainPanel_ = new MarineNaviMainPanel(notebook, this, dependencies);
  diagnosticPanel_ = new DiagnosticPanel(notebook);

  notebook->AddPage(mainPanel_, _("Main"));
  notebook->AddPage(diagnosticPanel_, _("Diagnostic"));

  wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
  mainSizer->Add(notebook, 1, wxEXPAND | wxALL, 5);

  SetSizerAndFit(mainSizer);
}

void MarineNaviMainDlg::UpdateDiagnostic(
    const entities::Diagnostic& diagnostic) {
  fprintf(stderr, "In call\n");
  diagnosticPanel_->UpdateDiagnostic(diagnostic);
}

}  // namespace MarineNavi::dialogs
