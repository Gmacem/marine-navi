#include "Utils.h"

#include <iomanip>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

namespace MarineNavi {
namespace Utils {

time_t ParseDate(const std::string& repr, const std::string& format) {
  struct tm tm;
  strptime(repr.data(), format.data(), &tm);
  return mktime(&tm);
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

}  // namespace Utils
}  // namespace MarineNavi
