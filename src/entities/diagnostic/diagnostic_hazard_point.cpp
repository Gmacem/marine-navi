#include "diagnostic_hazard_point.h"

#include <sstream>

#include "common/utils.h"

namespace marine_navi::entities::diagnostic {

std::string DiagnosticHazardPoint::GetMessage() const {
    std::ostringstream ss;
    ss << "Diagnostic hazard point detected at location (" << location_.Lat << ", " << location_.Lon
                  << ") on " << common::ToString(check_time_)
                  << ", with expected troubles at " << common::ToString(expected_time_of_troubles_)
                  << ", with reason: " << reason_ << ".";
    return ss.str();
}


DiagnosticHazardPoint MakeDepthHazardPoint(
    common::Point location,
    time_t check_time,
    time_t expected_time_of_troubles,
    double depth) {

    return DiagnosticHazardPoint(
        location,
        check_time,
        expected_time_of_troubles,
        common::StringFormat("dangerous depth %lf", depth)
    );
}

DiagnosticHazardPoint MakeHighWavesHazardPoint(
    common::Point location,
    time_t check_time,
    time_t expected_time_of_troubles,
    double wave_height) {

    return DiagnosticHazardPoint(
        location,
        check_time,
        expected_time_of_troubles,
        common::StringFormat("dangerous wave height %lf", wave_height)
    );
}

} // namespace marine_navi::entities::diagnostic
