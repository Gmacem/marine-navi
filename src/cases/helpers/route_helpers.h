#pragma once

#include "entities/ship.h"

namespace marine_navi::cases::helpers {

double GetSpeed(const entities::ShipPerformanceInfo& info, const double wave_height);

}  // namespace marine_navi::cases::helpers
