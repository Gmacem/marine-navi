
#include "Diagnostic.h"

namespace marine_navi::entities {

namespace {
std::string ToString(Diagnostic::ReasonType reason) {
  switch (reason) {
    case Diagnostic::ReasonType::kHighWaves: {
      return "High waves";
    }
    case Diagnostic::ReasonType::kNotDeep: {
      return "Not deep enough";
    }
    default: {
      throw std::runtime_error("Unknown reason type");
    }
  }
}
}  // namespace

Diagnostic CreateHighWavesDiagnostic(Utils::Point location,
                                     Utils::Point nearest, double expectedHigh,
                                     std::string source,
                                     time_t expectedTimeOfTroubles) {
  std::stringstream description;
  description << "Nearest forecast point: " << nearest.Lat << " " << nearest.Lon
              << '\n'
              << "\tExpected high: " << expectedHigh << '\n';
  return Diagnostic{
      .Location = location,
      .Type = Diagnostic::ReasonType::kHighWaves,
      .Description = description.str(),
      .Source = source,
      .CheckTime = std::time(0),
      .ExpectedTimeOfTroubles = expectedTimeOfTroubles,
  };
}

Diagnostic CreateNotDeepDiagnostic(Utils::Point location, Utils::Point nearest,
                                   double depth, std::string source,
                                   time_t expectedTimeOfTroubles) {
  std::stringstream description;
  description << "Nearest depth point: " << nearest.Lat << " " << nearest.Lon
              << '\n'
              << "\tDepth: " << depth << '\n';
  return Diagnostic{
      .Location = location,
      .Type = Diagnostic::ReasonType::kNotDeep,
      .Description = description.str(),
      .Source = source,
      .CheckTime = std::time(0),
      .ExpectedTimeOfTroubles = expectedTimeOfTroubles,
  };
}

std::string GetDiagnosticMessage(const Diagnostic& diagnostic) {
  std::stringstream ss;
  ss << "Possible problems at the point: " << diagnostic.Location.Lat << " "
     << diagnostic.Location.Lon << '\n';
  ss << "Type: " << entities::ToString(diagnostic.Type) << '\n';
  ss << "Source: " << diagnostic.Source << '\n';
  ss << "Description: " << diagnostic.Description << '\n';
  return ss.str();
}

}  // namespace marine_navi::entities
