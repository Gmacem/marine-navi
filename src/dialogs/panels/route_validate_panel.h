#pragma once

#include <string>
#include <optional>

#include <wx/datectrl.h>
#include <wx/timectrl.h>
#include <wx/wx.h>

#include "dialogs/panels/diagnostic_panel.h"
#include "dependencies.h"

namespace marine_navi::dialogs::panels {

class RouteValidatePanel : public wxPanel {
public:
  RouteValidatePanel(wxWindow* parent, DiagnosticPanel* diagnostic_panel,
                      const Dependencies& dependencies);
  ~RouteValidatePanel() override { UnbindEvents(); }

private:
  void CreateControls();
  wxTextCtrl* CreateLabeledTextCtrl(wxSizer* sizer, const wxString& label, std::optional<wxString> default_value = std::nullopt);
  wxTextCtrl* CreateBrowseFileTextCtrl(wxButton* browse_button, wxSizer* sizer,
                                       const wxString& label);
  void BindEvents();
  void UnbindEvents();
  void OnCheckPathClicked(wxCommandEvent&);
  void OnLoadForecastsClicked(wxCommandEvent&);
  void OnBrowseDepthClicked(wxCommandEvent&);
  void OnRefreshRouteList(wxCommandEvent&);

private:
  DiagnosticPanel* diagnostic_panel_;
  wxWindow* canvas_window_;
  std::shared_ptr<marine_navi::cases::MarineRouteScanner> marine_route_scanner_;
  std::shared_ptr<marine_navi::cases::ForecastsLoader> forecasts_loader_;

  wxListBox* route_list_box_;
  wxTextCtrl* c_route_name_;

  wxTextCtrl* c_danger_height_;
  wxTextCtrl* c_engine_power_;
  wxTextCtrl* c_displacement_;
  wxTextCtrl* c_length_;
  wxTextCtrl* c_fullness_;
  wxTextCtrl* c_speed_;
  wxTextCtrl* c_ship_draft_;

  wxDatePickerCtrl* d_route_date_;
  wxTimePickerCtrl* t_route_time_;
  wxTextCtrl* c_depth_file_;

  wxButton* b_browse_depth_file_button_;
  wxButton* b_scan_route_;
  wxButton* b_load_forecasts_;
  wxButton* b_refresh_route_list_;
};


} // namespace marine_navi::dialogs:panels
