#include "utils.h"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

namespace marine_navi::common {

std::string TrimSpace(const std::string& str) {
    const auto str_begin = std::find_if_not(str.begin(), str.end(), [](unsigned char ch) {
        return std::isspace(ch);
    });
    const auto str_end = std::find_if_not(str.rbegin(), str.rend(), [](unsigned char ch) {
        return std::isspace(ch);
    }).base();
    if (str_begin >= str_end) {
        return "";
    }
    return std::string(str_begin, str_end);
}

time_t ParseDate(const std::string& repr, const std::string& format) {
  struct tm tm;
  std::istringstream ss(repr);
  ss >> std::get_time(&tm, format.data());
  return timegm(&tm);
}

std::string ToString(time_t t, const std::string& format) {
  char buffer[100];
  struct tm* tm = gmtime(&t);
  strftime(buffer, sizeof(buffer), format.data(), tm);
  return buffer;
}

std::string CurrentFormattedTime(const std::string& format) {
  auto t = std::time(nullptr);
  auto tm = *std::gmtime(&t);
  std::ostringstream oss;
  oss << std::put_time(&tm, format.data());
  return oss.str();
}

time_t GetCurrentTime() {
  return std::time(nullptr);
}

}  // namespace marine_navi::common
