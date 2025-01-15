#include "depart_time_input.h"

namespace marine_navi::dialogs::panels {

namespace {
time_t GetTimeFromCtrls(wxDatePickerCtrl* date, wxTimePickerCtrl* time) {
  wxDateTime dateValue = date->GetValue();
  wxDateTime timeValue = time->GetValue();

  dateValue.SetHour(timeValue.GetHour());
  dateValue.SetMinute(timeValue.GetMinute());
  dateValue.SetSecond(timeValue.GetSecond());
  dateValue.SetMillisecond(timeValue.GetMillisecond());

  struct tm tmDateTime;

  tmDateTime.tm_year = dateValue.GetYear() - 1900;
  tmDateTime.tm_mon = dateValue.GetMonth();
  tmDateTime.tm_mday = dateValue.GetDay();
  tmDateTime.tm_hour = dateValue.GetHour();
  tmDateTime.tm_min = dateValue.GetMinute();
  tmDateTime.tm_sec = dateValue.GetSecond();
  tmDateTime.tm_isdst = -1;

  return mktime(&tmDateTime);
}
} // namespace

DepartTimeInput::DepartTimeInput(wxWindow* parent) : wxPanel(parent) {
  wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer* datetime = new wxBoxSizer(wxHORIZONTAL);
  wxBoxSizer* s_departure_date = new wxBoxSizer(wxVERTICAL);
  s_departure_date->Add(
      new wxStaticText(this, wxID_ANY, _("Departure date")));
  d_route_date_ = new wxDatePickerCtrl(this, wxID_ANY);
  s_departure_date->Add(d_route_date_);

  wxBoxSizer* s_departure_time = new wxBoxSizer(wxVERTICAL);
  s_departure_time->Add(
      new wxStaticText(this, wxID_ANY, _("Departure time")));
  t_route_time_ = new wxTimePickerCtrl(this, wxID_ANY);
  s_departure_time->Add(t_route_time_);

  datetime->Add(s_departure_date, 1, wxALL, 5);
  datetime->Add(s_departure_time, 1, wxALL, 5);
  main_sizer->Add(datetime);

  SetSizerAndFit(main_sizer);
  Centre(wxBOTH);
}

time_t DepartTimeInput::GetTime() const {
  return GetTimeFromCtrls(d_route_date_, t_route_time_);
}

} // namespace marine_navi::dialogs::panels