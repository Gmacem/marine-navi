#include "geom.h"

#include <algorithm>
#include <stdexcept>
#include <sstream>

#include "common/utils.h"

namespace marine_navi::common {

namespace {
constexpr double EARTH_RADIUS_METERS = 6371000.0; // Earth's radius in meters
const double DEG_TO_RAD = M_PI / 180.0;

double deg2rad(double degrees) {
    return degrees * DEG_TO_RAD;
}

int sign(double x) {
    if (std::abs(x) < kEps) return 0;
    return (x < 0) ? -1 : 1;
}

} // namespace

double& Point::X() { return Lon; }

double& Point::Y() { return Lat; }

const double& Point::X() const { return Lon; }

const double& Point::Y() const { return Lat; }

Point Point::Rotate(double alpha) const {
    double net_lat = Lat * cos(alpha) - Lon * sin(alpha);
    double new_lon = Lat * sin(alpha) + Lon * cos(alpha);
    return Point{net_lat, new_lon};
}


Point operator+(const Point& lhs, const Point& rhs) {
  return Point{lhs.Lat + rhs.Lat, lhs.Lon + rhs.Lon};
}

Point operator-(const Point& lhs, const Point& rhs) {
  return Point{lhs.Lat - rhs.Lat, lhs.Lon - rhs.Lon};
}

Point operator*(const Point& p, double x) {
  return Point{p.Lat * x, p.Lon * x};
}

Point operator*(double x, const Point& p) {
  return Point{p.Lat * x, p.Lon * x};
}

double DotProduct(const Point& p1, const Point& p2) {
    return p1.X() * p2.X() + p1.Y() * p2.Y();
}

double CrossProduct(const Point& p1, const Point& p2) {
    return p1.X() * p2.Y() - p1.Y() * p2.X();
}


bool IsInsideOfAngle(const Point a, const Point b, const Point c) {
  int cross_ab = sign(CrossProduct(a, b));
  int cross_bc = sign(CrossProduct(b, c));

  return cross_ab == cross_bc || cross_ab == 0 || cross_bc == 0;
}

double GetHaversineDistance(Point lhs, Point rhs) {
    double phi1 = deg2rad(lhs.Lat);
    double phi2 = deg2rad(rhs.Lat);
    double delta_phi = deg2rad(rhs.Lat - lhs.Lat);
    double delta_lambda = deg2rad(rhs.Lon - lhs.Lon);

    double a = std::sin(delta_phi / 2.0) * std::sin(delta_phi / 2.0) +
               std::cos(phi1) * std::cos(phi2) *
               std::sin(delta_lambda / 2.0) * std::sin(delta_lambda / 2.0);

    double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
    double distance = EARTH_RADIUS_METERS * c;

    return distance;
}

double GetHaversineDistance(const Segment& segment) {
  return GetHaversineDistance(segment.Start, segment.End);
}

Point Point::FromWktString(const std::string& wkt) {
  const std::string input = ::marine_navi::common::TrimSpace(wkt);
  if (input.rfind("POINT(", 0) != 0) {
    throw std::runtime_error("invalid wkt string " + wkt);
  }
  size_t start = input.find('(');
  size_t end = input.find(')', start);
  if (start == std::string::npos || end == std::string::npos) {
    throw std::runtime_error("invalid wkt string " + wkt);
  }
  std::string coord_str = input.substr(start + 1, end - start - 1);
  std::replace(coord_str.begin(), coord_str.end(), ',', ' ');

  Point result;

  std::istringstream ss(coord_str);
  if (!(ss >> result.Lon >> result.Lat)) {
    throw std::runtime_error("invalid wkt string " + wkt);
  }
  return result;
}

Segment Segment::FromWktString(const std::string& wkt) {
  const std::string input = ::marine_navi::common::TrimSpace(wkt);
  if (input.rfind("LINESTRING(", 0) != 0) {
    throw std::runtime_error("invalid wkt string " + wkt);
  }
  size_t start = input.find('(');
  size_t end = input.find(')', start);
  if (start == std::string::npos || end == std::string::npos) {
    throw std::runtime_error("invalid wkt string " + wkt);
  }
  std::string coord_str = input.substr(start + 1, end - start - 1);
  std::replace(coord_str.begin(), coord_str.end(), ',', ' ');

  Segment result;

  std::istringstream ss(coord_str);

  if (!(ss >> result.Start.Lon >> result.Start.Lat >> result.End.Lon >> result.End.Lat)) {
    throw std::runtime_error("invalid wkt string " + wkt);
  }
  return result;
}

} // namespace marine_navi::common
