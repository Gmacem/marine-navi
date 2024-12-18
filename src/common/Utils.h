#pragma once

#include <ctime>
#include <memory>
#include <stdexcept>
#include <string>
#include <time.h>

namespace marine_navi {
namespace Utils {

struct Point {
  double& X();
  double& Y();
  const double& X() const;
  const double& Y() const;

  double Lat;
  double Lon;
};

Point operator+(const Point& lhs, const Point& rhs);
Point operator-(const Point& lhs, const Point& rhs);
Point operator*(const Point& p, double x);
Point operator*(double x, const Point& p);

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

}  // namespace Utils
}  // namespace marine_navi