#pragma once

#include "Dependencies.h"

#include "entities/Diagnostic.h"

#include <wx/datectrl.h>
#include <wx/dialog.h>
#include <wx/notebook.h>
#include <wx/textctrl.h>
#include <wx/timectrl.h>
#include <wx/wx.h>

#include <functional>
#include <unordered_map>
#include <vector>

namespace MarineNavi::dialogs {

class MarineNaviMainDlg;

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

class MarineNaviMainPanel : public wxPanel {
public:
  MarineNaviMainPanel(wxWindow* parent, MarineNaviMainDlg* dlg,
                      const Dependencies& dependencies);
  ~MarineNaviMainPanel() override { UnbindEvents(); }

private:
  void CreateControls();
  wxTextCtrl* CreateLabeledTextCtrl(wxSizer* sizer, const wxString& label);
  wxTextCtrl* CreateBrowseFileTextCtrl(wxButton* browseButton, wxSizer* sizer,
                                       const wxString& label);
  void BindEvents();
  void UnbindEvents();
  void OnCheckPathClicked(wxCommandEvent&);
  void OnLoadForecastsClicked(wxCommandEvent&);
  void OnBrowseDepthClicked(wxCommandEvent&);
  void OnRefreshRouteList(wxCommandEvent&);

private:
  MarineNaviMainDlg* dlg_;
  wxWindow* canvasWindow_;
  std::shared_ptr<MarineNavi::cases::CheckPathCase> checkPathCase_;
  std::shared_ptr<MarineNavi::cases::ForecastsLoader> forecastsLoader_;

  wxListBox* routeListBox_;
  wxTextCtrl* cRouteName_;
  wxTextCtrl* cSpeed_;
  wxTextCtrl* cShipDraft_;
  wxTextCtrl* cMaxWave_;
  wxDatePickerCtrl* dRouteDate_;
  wxTimePickerCtrl* tRouteTime_;
  wxTextCtrl* cDepthFile_;

  wxButton* bBrowseDepthFileButton_;
  wxButton* bCheckPath_;
  wxButton* bLoadForecasts_;
  wxButton* bRefreshRouteList_;
};

class DiagnosticPanel : public wxPanel {
public:
  DiagnosticPanel(wxWindow* parent);

  void UpdateDiagnostic(const entities::Diagnostic& diagnostic);

private:
  wxTextCtrl* cDiagnosticMessage_;
};

class MarineNaviMainDlg : public MarineNaviDlgBase {
public:
  MarineNaviMainDlg(wxWindow* parent, wxWindowID id, const wxString& title,
                    const wxPoint& pos, const wxSize& size,
                    const Dependencies& dependencies);

  void UpdateDiagnostic(const entities::Diagnostic& diagnostic);

private:
  MarineNavi::Dependencies deps_;
  wxWindow* parentWindow_;
  std::shared_ptr<MarineNavi::dialogs::MarineNaviMainDlg> dlg_;
  std::shared_ptr<MarineNavi::RenderOverlay> renderOverlay_;
};

}  // namespace MarineNavi::dialogs
