#pragma once

#include <Dependencies.h>

#include <wx/wx.h>
#include <wx/notebook.h>

#include <functional>
#include <unordered_map>
#include <vector>

namespace marine_navi {

enum class EventType {
    kClose,
};

class MarineNaviDlgBase : public wxDialog {
public:
    MarineNaviDlgBase(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size);
    virtual ~MarineNaviDlgBase();

    void Register(std::function<void(void)> func, EventType event);

protected:
    void OnClose(wxCloseEvent& event);

private:
    std::unordered_map<EventType, std::vector<std::function<void(void)>>> callbacks_;
};

// Main panel for the first tab
class MarineNaviMainPanel : public wxPanel {
public:
    MarineNaviMainPanel(wxWindow* parent, const Dependencies& dependencies);
    ~MarineNaviMainPanel() override;

private:
    void CreateControls();
    wxTextCtrl* CreateLabeledTextCtrl(wxFlexGridSizer* sizer, const wxString& label);
    void BindEvents();
    void UnbindEvents();
    void OnCheckPathClicked(wxCommandEvent&);
    void OnLoadForecastsClicked(wxCommandEvent&);

private:
    wxWindow* canvasWindow_;
    std::shared_ptr<cases::CheckPathCase> checkPathCase_;
    std::shared_ptr<cases::ForecastsLoader> forecastsLoader_;

    wxTextCtrl* cStartLat_;
    wxTextCtrl* cStartLon_;
    wxTextCtrl* cEndLat_;
    wxTextCtrl* cEndLon_;
    wxTextCtrl* cShipDraft_;
    wxTextCtrl* cMaxWave_;
    wxTextCtrl* cPathToFile_;
    wxButton* bCheckPath_;
    wxButton* bLoadForecasts_;
};

class MarineNaviMainDlg : public MarineNaviDlgBase {
public:
    MarineNaviMainDlg(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, const Dependencies& dependencies);
    ~MarineNaviMainDlg();
};

}  // namespace marine_navi
