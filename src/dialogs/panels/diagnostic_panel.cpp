#include "diagnostic_panel.h"

#include <wx/log.h>
#include <wx/sizer.h>

namespace marine_navi::dialogs::panels {

DiagnosticPanel::DiagnosticPanel(wxWindow* parent) : wxPanel(parent) {
  c_diagnostic_message_ =
      new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
                     wxDefaultSize, wxTE_READONLY | wxTE_MULTILINE);
  wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
  sizer->Add(c_diagnostic_message_, 1, wxEXPAND | wxALL, 5);
  SetSizerAndFit(sizer);
}

void DiagnosticPanel::UpdateDiagnostic(const entities::RouteValidateDiagnostic& diagnostic) {
  auto message = entities::GetDiagnosticMessage(diagnostic);
  c_diagnostic_message_->SetValue(message);
}

} // namespace marine_navi::dialogs::panels
