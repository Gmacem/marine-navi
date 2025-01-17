#include "marine_math.h"

namespace marine_navi::common {

double CalculateVelocityRatio(double N, double D, double L, double delta, double h3_percent) {
    double gamma1 = std::pow(N / D, -1.14) - 2;
    double gamma2 = std::pow(delta / (1.143 - 1.425 * (N / D)), 4.7);
    double gamma3 = 1.25 * std::exp(-1.48 * (10 * h3_percent / L));
    double gamma = gamma1 * gamma2 * gamma3;

    double velocityRatio = std::exp(-gamma * std::pow(10 * h3_percent / L, 2));

    return velocityRatio;
}

double KnotsToMetersPerSecond(double knots) {
    return knots * 0.514444;
}

double FeetToMeters(double ft) {
    return ft * 0.3048;
}

double CalculateSteeringAngle(double v, double v_nominal) {
    const double degrees_to_radians = M_PI / 180.0;
    double alpha_degrees = 1.0 * (v / v_nominal);
    double alpha_radians = alpha_degrees * degrees_to_radians;

    return std::min(alpha_radians, M_PI);
}

} // namespace marine_navi::common
