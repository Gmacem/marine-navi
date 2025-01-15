#pragma once

#include <wx/datectrl.h>
#include <wx/timectrl.h>
#include <wx/sizer.h>
#include <wx/wx.h>

namespace marine_navi::dialogs::panels {

class DepartTimeInput : public wxPanel {
public:
  DepartTimeInput(wxWindow* parent);

  time_t GetTime() const;

private:
  wxDatePickerCtrl* d_route_date_;
  wxTimePickerCtrl* t_route_time_;
};

}  // namespace marine_navi::dialogs::panels