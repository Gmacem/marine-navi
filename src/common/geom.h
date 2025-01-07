#pragma once

#include <cmath>
#include <string>
#include <vector>

namespace marine_navi::common {

const double kEps = 1e-5;

struct Point {
  double& X();
  double& Y();
  const double& X() const;
  const double& Y() const;
  Point Rotate(double alpha) const;

  static Point FromWktString(const std::string& wkt);

  double Lat;
  double Lon;
};

Point operator+(const Point& lhs, const Point& rhs);
Point operator-(const Point& lhs, const Point& rhs);
Point operator*(const Point& p, double x);
Point operator*(double x, const Point& p);

double DotProduct(const Point& p1, const Point& p2);
double CrossProduct(const Point& p1, const Point& p2);

bool IsInsideOfAngle(const Point a, const Point b, const Point c);

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
