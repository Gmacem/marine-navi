#pragma once

#include <string>

namespace marine_navi::entities::diagnostic {
    
class IDiagnosticEvent {
public:
  virtual std::string GetMessage() const = 0;

  virtual ~IDiagnosticEvent() = default;
};

} // namespace marine_navi::entities::diagnostic
