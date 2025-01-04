#pragma once

#include <wx/panel.h>
#include <wx/textctrl.h>

#include "entities/diagnostic.h"

namespace marine_navi::dialogs::panels {

class DiagnosticPanel : public wxPanel {
public:
  DiagnosticPanel(wxWindow* parent);

  void UpdateDiagnostic(const entities::RouteValidateDiagnostic& diagnostic);

private:
  wxTextCtrl* c_diagnostic_message_;
};

} // namespace marine_navi::dialogs::panels
