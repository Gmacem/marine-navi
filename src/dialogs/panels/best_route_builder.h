#pragma once

#include <string>
#include <optional>

#include <wx/datectrl.h>
#include <wx/timectrl.h>
#include <wx/wx.h>

#include "dialogs/panels/ship_info_panel.h"

#include "dependencies.h"

namespace marine_navi::dialogs::panels {

class BestRouteBuilderPanel : public wxPanel {
public:
  BestRouteBuilderPanel(wxWindow* parent, const Dependencies& dependencies);

private:
    
};

} // namespace marine_navi::dialogs:panels
