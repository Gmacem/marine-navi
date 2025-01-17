
#include "diagnostic.h"

namespace marine_navi::entities::diagnostic {

namespace {

using Point = common::Point;

} // namespace

std::string GetDiagnosticMessage(const RouteValidateDiagnostic& diagnostic) {
  if (diagnostic.result == RouteValidateDiagnostic::DiagnosticResultType::kOk) {
    return "OK";
  }

  std::stringstream ss;
  for(const auto& point : diagnostic.hazard_points) {
    ss << point.GetMessage() << '\n';
  }

  return ss.str();
}

}  // namespace marine_navi::entities::diagnostic
