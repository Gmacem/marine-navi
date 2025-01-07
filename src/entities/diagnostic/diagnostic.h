#pragma once

#include <ctime>
#include <sstream>
#include <variant>

#include "common/geom.h"
#include "common/utils.h"

#include "entities/diagnostic/diagnostic_hazard_point.h"
#include "entities/diagnostic/diagnostic_event.h"

namespace marine_navi::entities::diagnostic {

struct RouteValidateDiagnostic {
  enum class DiagnosticResultType {
    kOk,
    kWarning,
  };

  DiagnosticResultType result;
  std::vector<DiagnosticHazardPoint> hazard_points;
};

std::string GetDiagnosticMessage(const RouteValidateDiagnostic& diagnostic);

}  // namespace marine_navi::entities
