#include "ship_info_panel.h"

#include "common/marine_math.h"
#include "dialogs/panels/helpers.h"

namespace marine_navi::dialogs::panels {

namespace {
bool CheckIsEmpty(wxTextCtrl* txt, wxString message) {
  if (txt->IsEmpty()) {
    wxMessageBox(message, "Error", wxOK | wxICON_ERROR);
    return true;
  }
  return false;
}
} // namespace

ShipInfoPanel::ShipInfoPanel(wxWindow* parent): ShipInfoPanel(parent, new wxBoxSizer(wxVERTICAL)) {}

ShipInfoPanel::ShipInfoPanel(wxWindow* parent, wxSizer* sizer): wxPanel(parent) {
    sizer_ = sizer;
    c_danger_height_ = CreateLabeledTextCtrl(this, sizer, _("Danger height, m"), "2");
    c_engine_power_ = CreateLabeledTextCtrl(this, sizer, _("Engine power, kw"), "1000");
    c_displacement_ = CreateLabeledTextCtrl(this, sizer, _("Displacement, ton"), "5000");
    c_length_ = CreateLabeledTextCtrl(this, sizer, _("Ship length, m"), "100");
    c_fullness_ = CreateLabeledTextCtrl(this, sizer, _("Block coefficient"), "0.8");
    c_speed_ = CreateLabeledTextCtrl(this, sizer, _("Speed, knot"), "2");
    c_ship_draft_ = CreateLabeledTextCtrl(this, sizer, _("Ship draft, m"), "1");
}

entities::ShipPerformanceInfo ShipInfoPanel::GetShipInfo() {
  if (CheckIsEmpty(c_speed_, _("Please specify speed"))) {
    return entities::ShipPerformanceInfo{};
  }

  auto parseDouble = [](const wxString& str) -> std::optional<double> {
    double result;
    if (!str.ToDouble(&result)) {
      return std::nullopt;
    }
    return result;
  };

  entities::ShipPerformanceInfo result;

  result.DangerHeight = parseDouble(c_danger_height_->GetValue().mb_str());
  result.EnginePower = parseDouble(c_engine_power_->GetValue().mb_str());
  result.Displacement = parseDouble(c_displacement_->GetValue().mb_str());
  result.Length = parseDouble(c_length_->GetValue().mb_str());
  result.Fullness = parseDouble(c_fullness_->GetValue().mb_str());
  result.Speed = parseDouble(c_speed_->GetValue().mb_str());
  if (result.Speed.has_value()) {
    result.Speed = common::KnotsToMetersPerSecond(result.Speed.value());
  } else {
    wxMessageBox("Invalid speed", "Error", wxOK | wxICON_ERROR);
    return entities::ShipPerformanceInfo{};
  }
  result.ShipDraft = parseDouble(c_ship_draft_->GetValue().mb_str());
  if (result.ShipDraft.has_value()) {
    result.ShipDraft = common::FeetToMeters(result.ShipDraft.value());
  }
  return result;
}

} // namespace marine_navi::dialogs::panels