#pragma once

#include <cmath>
#include <iostream>

namespace marine_navi::common {

/**
 * @brief Calculates the velocity ratio (v/v_sw) based on empirical formulas.
 * 
 * @param N Engine power of the ship, in kilowatts (kW).
 * @param D Displacement of the ship, in tons.
 * @param L Length of the ship, in meters (m).
 * @param delta Fullness of the hull (dimensionless ratio).
 * @param h3_percent Wave intensity or 3%-exceedance wave height (dimensionless).
 * @param draft Draft of the ship, in feet (ft).
 * @return The velocity ratio (v/v_sw) as a dimensionless value.
 */   
double CalculateVelocityRatio(double N, double D, double L, double delta, double h3_percent);

double KnotsToMetersPerSecond(double knots);

} // namespace marine_navi::common
