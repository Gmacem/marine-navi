#pragma once

#include <wx/wx.h>

#include "dependencies.h"
#include "cases/safe_point_manager.h"

namespace marine_navi::dialogs::panels {
class SafePointManagerPanel : public wxPanel {
public:
  SafePointManagerPanel(wxWindow* parent, const Dependencies& dependencies);
  ~SafePointManagerPanel() override { UnbindEvents(); }

  void BindEvents();
  void UnbindEvents();
  void OnRefreshSafePointList(wxCommandEvent&);
  void OnAddSafePoint(wxCommandEvent&);

private:
  wxListBox* waypoint_list_box_;
  wxButton* b_refresh_waypoint_list_;
  wxButton* b_add_safe_point_;

  std::shared_ptr<cases::SafePointManager> safe_point_manager_;
};

}  // namespace marine_navi::dialogs::panels
