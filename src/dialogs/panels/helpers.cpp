#include "helpers.h"

#include <wx/sizer.h>
#include <wx/valnum.h>
#include <wx/wx.h>

namespace marine_navi::dialogs::panels {

wxTextCtrl* CreateLabeledTextCtrl(wxWindow* parent, wxSizer* sizer,
                                  const wxString& label,
                                  std::optional<wxString> default_value) {
  wxFloatingPointValidator<double> validator(2, nullptr, wxNUM_VAL_DEFAULT);
  validator.SetRange(-1e5, 1e5);
  sizer->Add(new wxStaticText(parent, wxID_ANY, label));

  wxTextCtrl* textCtrl =
      new wxTextCtrl(parent, wxID_ANY, default_value.value_or(wxEmptyString),
                     wxDefaultPosition, wxDefaultSize, 0, validator);
  sizer->Add(textCtrl, 0, wxALL, 5);
  return textCtrl;
}

wxTextCtrl* CreateBrowseFileTextCtrl(wxWindow* parent, wxButton* browseButton,
                                     wxSizer* sizer, const wxString& label) {
  wxBoxSizer* fileSizer = new wxBoxSizer(wxHORIZONTAL);
  fileSizer->Add(new wxStaticText(parent, wxID_ANY, label));
  wxTextCtrl* textCtrl = new wxTextCtrl(parent, wxID_ANY, "");
  fileSizer->Add(textCtrl, 1, wxALL, 5);
  fileSizer->Add(browseButton, 0, wxALL, 5);
  sizer->Add(fileSizer, 1, wxALL, 5);
  return textCtrl;
}

}  // namespace marine_navi::dialogs::panels
