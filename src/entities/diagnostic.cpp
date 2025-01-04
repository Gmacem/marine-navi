
#include "diagnostic.h"

namespace marine_navi::entities {

namespace {
std::string ToString(RouteValidateDiagnostic::ReasonType reason) {
  switch (reason) {
    case RouteValidateDiagnostic::ReasonType::kHighWaves: {
      return "High waves";
    }
    case RouteValidateDiagnostic::ReasonType::kNotDeep: {
      return "Not deep enough";
    }
    default: {
      throw std::runtime_error("Unknown reason type");
    }
  }
}
}  // namespace

RouteValidateDiagnostic CreateHighWavesDiagnostic(Utils::Point location,
                                     Utils::Point nearest, double expectedHigh,
                                     std::string source,
                                     time_t expectedTimeOfTroubles) {
  std::stringstream description;
  description << "Nearest forecast point: " << nearest.Lat << " " << nearest.Lon
              << '\n'
              << "\tExpected high: " << expectedHigh << '\n';
  return RouteValidateDiagnostic{
      .Location = location,
      .Type = RouteValidateDiagnostic::ReasonType::kHighWaves,
      .Description = description.str(),
      .Source = source,
      .CheckTime = std::time(0),
      .ExpectedTimeOfTroubles = expectedTimeOfTroubles,
  };
}

RouteValidateDiagnostic CreateNotDeepDiagnostic(Utils::Point location, Utils::Point nearest,
                                   double depth, std::string source,
                                   time_t expectedTimeOfTroubles) {
  std::stringstream description;
  description << "Nearest depth point: " << nearest.Lat << " " << nearest.Lon
              << '\n'
              << "\tDepth: " << depth << '\n';
  return RouteValidateDiagnostic{
      .Location = location,
      .Type = RouteValidateDiagnostic::ReasonType::kNotDeep,
      .Description = description.str(),
      .Source = source,
      .CheckTime = std::time(0),
      .ExpectedTimeOfTroubles = expectedTimeOfTroubles,
  };
}

std::string GetDiagnosticMessage(const RouteValidateDiagnostic& diagnostic) {
  std::stringstream ss;
  ss << "Possible problems at the point: " << diagnostic.Location.Lat << " "
     << diagnostic.Location.Lon << '\n';
  ss << "Type: " << entities::ToString(diagnostic.Type) << '\n';
  ss << "Source: " << diagnostic.Source << '\n';
  ss << "Description: " << diagnostic.Description << '\n';
  return ss.str();
}

}  // namespace marine_navi::entities
