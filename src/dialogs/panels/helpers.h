#pragma once

#include <string>
#include <optional>

#include <wx/button.h>
#include <wx/textctrl.h>

namespace marine_navi::dialogs::panels {

wxTextCtrl* CreateLabeledTextCtrl(wxWindow *parent, wxSizer* sizer, const wxString& label, std::optional<wxString> default_value = std::nullopt);
wxTextCtrl* CreateBrowseFileTextCtrl(wxWindow *parent, wxButton* browse_button, wxSizer* sizer,
                                    const wxString& label);

} // namespace marine_navi::dialogs::panels
