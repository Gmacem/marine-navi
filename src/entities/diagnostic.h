#pragma once

#include <ctime>
#include <sstream>

#include "common/utils.h"

namespace marine_navi::entities {

struct RouteValidateDiagnostic {
  enum class ReasonType {
    kNotDeep,
    kHighWaves,
  };

  Utils::Point Location;
  ReasonType Type;
  std::string Description;
  std::string Source;
  time_t CheckTime;
  time_t ExpectedTimeOfTroubles;
};

RouteValidateDiagnostic CreateHighWavesDiagnostic(Utils::Point location,
                                     Utils::Point nearest, double expectedHigh,
                                     std::string source,
                                     time_t expectedTimeOfTroubles);
RouteValidateDiagnostic CreateNotDeepDiagnostic(Utils::Point location, Utils::Point nearest,
                                   double depth, std::string source,
                                   time_t expectedTimeOfTroubles);

std::string GetDiagnosticMessage(const RouteValidateDiagnostic& diagnostic);
}  // namespace marine_navi::entities
