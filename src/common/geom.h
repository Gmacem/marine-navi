#pragma once

#include <cmath>
#include <string>
#include <vector>

namespace marine_navi::common {

struct Point {
  double& X();
  double& Y();
  const double& X() const;
  const double& Y() const;
  inline double Length() const { return sqrt(X() * X() + Y() * Y()); }

  static Point FromWktString(const std::string& wkt);

  double Lat;
  double Lon;
};

Point operator+(const Point& lhs, const Point& rhs);
Point operator-(const Point& lhs, const Point& rhs);
Point operator*(const Point& p, double x);
Point operator*(double x, const Point& p);

struct Segment {
  static Segment FromWktString(const std::string& wkt);

  Point Start;
  Point End;
};

struct Polyline {
  std::vector<Point> Points;
};

double GetHaversineDistance(Point lhs, Point rhs);
double GetHaversineDistance(const Segment& segment);

} // namespace marine_navi::common
