#pragma once

#include <string>
#include <time.h>

#include "common/geom.h"
#include "entities/depth_point.h"
#include "entities/diagnostic/diagnostic_event.h"

namespace marine_navi::entities::diagnostic {

class DiagnosticHazardPoint : public IDiagnosticEvent {
public:
  DiagnosticHazardPoint(common::Point location,
                        time_t check_time,
                        time_t expected_time_of_troubles,
                        std::string reason): 
    location_(location),
    check_time_(check_time),
    expected_time_of_troubles_(expected_time_of_troubles),
    reason_(reason) {
  }
  std::string GetMessage() const override;
  const common::Point GetLocation() const { return location_; }

  ~DiagnosticHazardPoint() = default;

private:
  common::Point location_;
  time_t check_time_;
  time_t expected_time_of_troubles_;
  std::string reason_;
};

DiagnosticHazardPoint MakeDepthHazardPoint(
    common::Point location,
    time_t check_time,
    time_t expected_time_of_troubles,
    double depth);

DiagnosticHazardPoint MakeHighWavesHazardPoint(
    common::Point location,
    time_t check_time,
    time_t expected_time_of_troubles,
    double wave_height);

} // namespace marine_navi::entities::diagnostic