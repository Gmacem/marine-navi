#pragma once

#include <wx/wx.h>

#include <entities/ship.h>

namespace marine_navi::dialogs::panels {

class ShipInfoPanel : public wxPanel {
public:
    ShipInfoPanel(wxWindow* parent);
    ShipInfoPanel(wxWindow* parent, wxSizer* sizer);

    entities::ShipPerformanceInfo GetShipInfo();

private:
  wxTextCtrl* c_danger_height_;
  wxTextCtrl* c_engine_power_;
  wxTextCtrl* c_displacement_;
  wxTextCtrl* c_length_;
  wxTextCtrl* c_fullness_;
  wxTextCtrl* c_speed_;
  wxTextCtrl* c_ship_draft_;

  wxSizer* sizer_;
};

} // namespace marine_navi::dialogs::panels
