#pragma once

#include <optional>
#include <vector>

#include <wx/wx.h>

#include "entities/route.h"

namespace marine_navi::dialogs::panels {

class SelectRoutePanel : public wxPanel {
public:
    SelectRoutePanel(wxWindow* parent);
    ~SelectRoutePanel() { UnbindRefreshList(); }

    std::shared_ptr<entities::Route> GetRoute() const;

protected:
    virtual void OnRefresh(wxCommandEvent&);

    void BindRefreshList();
    void UnbindRefreshList();

protected:
  wxListBox* list_box_;
  wxButton* b_refresh_list_;
};

} // namespace marine_navi::dialogs::panels
