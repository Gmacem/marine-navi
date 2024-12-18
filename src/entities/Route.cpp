#include "Route.h"

namespace marine_navi::entities {

double Route::GetDistance() const {
  double dist = 0;
  for (size_t i = 0; i + 1 < points_.size(); ++i) {
    dist += Utils::GetDistance(points_[i], points_[i + 1]);
  }
  return dist;
}

Utils::Point Route::GetPointFromStart(double len) {
  double pref = 0;
  for (size_t i = 0; i + 1 < points_.size(); ++i) {
    double dist = Utils::GetDistance(points_[i], points_[i + 1]);
    if (len <= pref + dist) {
      auto vec = points_[i + 1] - points_[i];
      double k = (len - pref) / dist;
      return points_[i] + vec * k;
    }
    pref += dist;
  }

  return points_.back();
}

}  // namespace marine_navi::entities