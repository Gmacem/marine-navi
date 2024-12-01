#pragma once

#include <common/Utils.h>

#include <ctime>
#include <sstream>

namespace MarineNavi::entities {

struct Diagnostic {
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

Diagnostic CreateHighWavesDiagnostic(Utils::Point location,
                                     Utils::Point nearest, double expectedHigh,
                                     std::string source,
                                     time_t expectedTimeOfTroubles);
Diagnostic CreateNotDeepDiagnostic(Utils::Point location, Utils::Point nearest,
                                   double depth, std::string source,
                                   time_t expectedTimeOfTroubles);

std::string GetDiagnosticMessage(const Diagnostic& diagnostic);
}  // namespace MarineNavi::entities