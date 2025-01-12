#pragma once 

#include <optional>

namespace marine_navi::entities {

struct ShipPerformanceInfo {
  std::optional<double> DangerHeight;
  std::optional<double> EnginePower;
  std::optional<double> Displacement;
  std::optional<double> Length;
  std::optional<double> Fullness;
  std::optional<double> Speed;
  std::optional<double> ShipDraft;
};

} // namespace marine_navi::entities
