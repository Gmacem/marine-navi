#include "MainDialog.h"

#include "cases/CheckPathCase.h"
#include "cases/ForecastsLoader.h"

#include <ocpn_plugin.h>

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/button.h>

#include <memory>

namespace marine_navi {

// Base dialog class
MarineNaviDlgBase::MarineNaviDlgBase(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size)
    : wxDialog(parent, id, title, pos, size, wxRESIZE_BORDER | wxDEFAULT_DIALOG_STYLE) {
    this->Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(MarineNaviDlgBase::OnClose));
}

void MarineNaviDlgBase::Register(std::function<void(void)> func, EventType event) {
    callbacks_[event].push_back(func);
}

MarineNaviDlgBase::~MarineNaviDlgBase() {
    this->Disconnect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(MarineNaviDlgBase::OnClose));
}

void MarineNaviDlgBase::OnClose(wxCloseEvent& event) {
    for (auto& callback : callbacks_[EventType::kClose]) {
        callback();
    }
}

// First tab panel class
MarineNaviMainPanel::MarineNaviMainPanel(wxWindow* parent, const Dependencies& dependencies)
    : wxPanel(parent),
        canvasWindow_(dependencies.OcpnCanvasWindow),
        checkPathCase_(dependencies.CheckPathCase),
        forecastsLoader_(dependencies.ForecastsLoader) {
    CreateControls();
    BindEvents();
}

MarineNaviMainPanel::~MarineNaviMainPanel() {
    UnbindEvents();
}

void MarineNaviMainPanel::CreateControls() {
    wxFlexGridSizer* fgSizer = new wxFlexGridSizer(2, 5, 0, 0);
    fgSizer->AddGrowableCol(1);
    fgSizer->SetFlexibleDirection(wxBOTH);

    cStartLat_ = CreateLabeledTextCtrl(fgSizer, _("Start point lat"));
    cStartLon_ = CreateLabeledTextCtrl(fgSizer, _("Start point lon"));
    cEndLat_ = CreateLabeledTextCtrl(fgSizer, _("End point lat"));
    cEndLon_ = CreateLabeledTextCtrl(fgSizer, _("End point lon"));
    cShipDraft_ = CreateLabeledTextCtrl(fgSizer, _("Ship draft"));
    cMaxWave_ = CreateLabeledTextCtrl(fgSizer, _("Max wave"));
    cPathToFile_ = CreateLabeledTextCtrl(fgSizer, _("Path to depth data file"));

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(fgSizer, 1, wxALL | wxEXPAND, 5);

    bCheckPath_ = new wxButton(this, wxID_ANY, _("Check path"));
    bLoadForecasts_ = new wxButton(this, wxID_ANY, _("Download forecasts"));
    mainSizer->Add(bCheckPath_, 0, wxALL | wxEXPAND, 5);
    mainSizer->Add(bLoadForecasts_, 0, wxALL | wxEXPAND, 5);

    SetSizerAndFit(mainSizer);
    Centre(wxBOTH);
}

wxTextCtrl* MarineNaviMainPanel::CreateLabeledTextCtrl(wxFlexGridSizer* sizer, const wxString& label) {
    sizer->Add(new wxStaticText(this, wxID_ANY, label));
    wxTextCtrl* textCtrl = new wxTextCtrl(this, wxID_ANY);
    sizer->Add(textCtrl, 0, wxALL | wxEXPAND, 5);
    return textCtrl;
}

void MarineNaviMainPanel::BindEvents() {
    bCheckPath_->Bind(wxEVT_BUTTON, &MarineNaviMainPanel::OnCheckPathClicked, this);
    bLoadForecasts_->Bind(wxEVT_BUTTON, &MarineNaviMainPanel::OnLoadForecastsClicked, this);
}

void MarineNaviMainPanel::UnbindEvents() {
    bCheckPath_->Unbind(wxEVT_BUTTON, &MarineNaviMainPanel::OnCheckPathClicked, this);
    bLoadForecasts_->Unbind(wxEVT_BUTTON, &MarineNaviMainPanel::OnLoadForecastsClicked, this);
}

void MarineNaviMainPanel::OnCheckPathClicked(wxCommandEvent&) {
    cases::PathData pathData;

    auto parseDouble = [](const wxString &str, double &value) {
        return str.ToDouble(&value);
    };

    if (!parseDouble(cStartLat_->GetValue(), pathData.Start.Lat) ||
        !parseDouble(cStartLon_->GetValue(), pathData.Start.Lon) ||
        !parseDouble(cEndLat_->GetValue(), pathData.End.Lat) ||
        !parseDouble(cEndLon_->GetValue(), pathData.End.Lon)) {
        printf("failed to parse start or end coordinates\n");
        return;
    }

    // if (!cShipDraft_->GetValue().empty() && !parseDouble(cShipDraft_->GetValue(), pathData.ShipDraft.v)) {
    //     printf("failed to parse ship draft\n");
    //     return;
    // }
    // if (!cMaxWave_->GetValue().empty() && !parseDouble(cMaxWave_->GetValue(), pathData.MaxWaveHeight)) {
    //     printf("failed to parse max wave\n");
    //     return;
    // }

    pathData.PathToDepthFile = cPathToFile_->GetValue();

    checkPathCase_->SetPathData(pathData);
    checkPathCase_->SetShow(true);
    checkPathCase_->CrossDetect();
    RequestRefresh(canvasWindow_);
}

void MarineNaviMainPanel::OnLoadForecastsClicked(wxCommandEvent&) {
    try {
        forecastsLoader_->Load();
    } catch (const std::exception& ex) {
        fprintf(stderr, "Failed to load forecasts: %s\n", ex.what());
    }
}

// Main dialog with tabs
MarineNaviMainDlg::MarineNaviMainDlg(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, const Dependencies& dependencies)
    : MarineNaviDlgBase(parent, id, title, pos, size) {
    wxNotebook* notebook = new wxNotebook(this, wxID_ANY);

    MarineNaviMainPanel* mainPanel = new MarineNaviMainPanel(notebook, dependencies);
    wxPanel* emptyPanel = new wxPanel(notebook, wxID_ANY);

    notebook->AddPage(mainPanel, _("Main"));
    notebook->AddPage(emptyPanel, _("Empty"));

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(notebook, 1, wxEXPAND | wxALL, 5);

    SetSizerAndFit(mainSizer);
}

MarineNaviMainDlg::~MarineNaviMainDlg() {}

}  // namespace marine_navi