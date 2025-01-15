#include "route_helpers.h"

#include "common/marine_math.h"

namespace marine_navi::cases::helpers {

double GetSpeed(const entities::ShipPerformanceInfo& info, const double wave_height) {
  if (
      !info.DangerHeight.has_value() ||
      !info.EnginePower.has_value() ||
      !info.Displacement.has_value() ||
      !info.Length.has_value() ||
      !info.Fullness.has_value() ||
      !info.ShipDraft.has_value()
  ) {
    return info.Speed.value();
  }
  auto r = common::CalculateVelocityRatio(
    info.EnginePower.value(),
    info.Displacement.value(),
    info.Length.value(),
    info.Fullness.value(),
    wave_height
  );

  return r * info.Speed.value();
}

} // namespace marine_navi::cases::helpers
