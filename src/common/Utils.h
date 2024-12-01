#pragma once

#include <cmath>
#include <ctime>
#include <memory>
#include <stdexcept>
#include <string>
#include <time.h>

namespace MarineNavi::Utils {

struct Point {
  inline double& X() { return Lon; }
  inline double& Y() { return Lat; }
  inline const double& X() const { return Lon; }
  inline const double& Y() const { return Lat; }
  inline double Length() const { return sqrt(Lat * Lat + Lon * Lon); }
  double Lat;
  double Lon;
};

inline Point operator+(const Point& lhs, const Point& rhs) {
  return Point{lhs.Lat + rhs.Lat, lhs.Lon + rhs.Lon};
}
inline Point operator-(const Point& lhs, const Point& rhs) {
  return Point{lhs.Lat - rhs.Lat, lhs.Lon - rhs.Lon};
}
inline Point operator*(const Point& p, double x) {
  return Point{p.Lat * x, p.Lon * x};
}
inline Point operator*(double x, const Point& p) { return p * x; }

inline double GetDistance(const Point& lhs, const Point& rhs) {
  return (rhs - lhs).Length();
}

time_t ParseDate(const std::string& repr,
                 const std::string& format = "%Y-%m-%d %H:%M:%S");

std::string ToString(time_t t, const std::string& format = "%Y-%m-%d %H:%M:%S");

template <typename... Args>
std::string StringFormat(const std::string& format, Args... args) {
  int size_s = std::snprintf(nullptr, 0, format.c_str(), args...) + 1;
  if (size_s <= 0) {
    throw std::runtime_error("Error during formatting.");
  }
  auto size = static_cast<size_t>(size_s);
  std::unique_ptr<char[]> buf(new char[size]);
  std::snprintf(buf.get(), size, format.c_str(), args...);
  return std::string(buf.get(), buf.get() + size - 1);
}

std::string CurrentFormattedTime(
    const std::string& format = "%Y-%m-%d %H:%M:%S");

}  // namespace MarineNavi::Utils