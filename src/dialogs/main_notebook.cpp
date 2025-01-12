#include "main_notebook.h"

#include <memory>

#include <wx/notebook.h>

#include <ocpn_plugin.h>


namespace marine_navi::dialogs {

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

MarineNaviMainDlg::MarineNaviMainDlg(wxWindow* parent, wxWindowID id,
                                     const wxString& title, const wxPoint& pos,
                                     const wxSize& size,
                                     const Dependencies& dependencies)
    : MarineNaviDlgBase(parent, id, title, pos, size) {
  wxNotebook* notebook = new wxNotebook(this, wxID_ANY);

  diagnostic_panel_ = new panels::DiagnosticPanel(notebook);
  route_validate_panel_ = new panels::RouteValidatePanel(notebook, diagnostic_panel_, dependencies);
  safe_point_manager_panel_ = new panels::SafePointManagerPanel(notebook, dependencies);

  notebook->AddPage(route_validate_panel_, _("Main"));
  notebook->AddPage(diagnostic_panel_, _("Diagnostic"));
  notebook->AddPage(safe_point_manager_panel_, _("Safe point manager"));

  wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
  mainSizer->Add(notebook, 1, wxEXPAND | wxALL, 5);

  SetSizerAndFit(mainSizer);
}

}  // namespace marine_navi::dialogs
