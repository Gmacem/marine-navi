#pragma once

#include <functional>
#include <unordered_map>
#include <vector>

#include "dependencies.h"
#include "dialogs/panels/diagnostic_panel.h"
#include "dialogs/panels/route_validate_panel.h"
#include "dialogs/panels/safe_point_manager_panel.h"

namespace marine_navi::dialogs {

class MarineNaviDlgBase : public wxDialog {
public:
  enum class EventType {
    kClose,
  };

  MarineNaviDlgBase(wxWindow* parent, wxWindowID id, const wxString& title,
                    const wxPoint& pos, const wxSize& size);
  virtual ~MarineNaviDlgBase();

  void Register(std::function<void(void)> func, EventType event);

protected:
  void OnClose(wxCloseEvent& event);

private:
  std::unordered_map<EventType, std::vector<std::function<void(void)>>>
      callbacks_;
};

class MarineNaviMainDlg : public MarineNaviDlgBase {
public:
  MarineNaviMainDlg(wxWindow* parent, wxWindowID id, const wxString& title,
                    const wxPoint& pos, const wxSize& size,
                    const Dependencies& dependencies);

private:
  panels::DiagnosticPanel* diagnostic_panel_;
  panels::RouteValidatePanel* route_validate_panel_;
  panels::SafePointManagerPanel* safe_point_manager_panel_;

};

}  // namespace marine_navi::dialogs
