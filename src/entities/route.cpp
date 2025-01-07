#include "route.h"

namespace marine_navi::entities {

Route::Route(std::vector<PlugIn_Waypoint> waypoints) : waypoints_(waypoints), total_distance_(0) {
  for (size_t i = 0; i < waypoints_.size(); ++i) {
    const auto waypoint = common::Point{
      waypoints_[i].m_lat,
      waypoints_[i].m_lon
    };

    if (i != 0) {
      total_distance_ += common::GetHaversineDistance(
        points_.back().point, waypoint
      );
    }
    points_.push_back(RoutePoint{waypoint, i, total_distance_, 0});
  }

  for(size_t i = 0; i + 1 < waypoints.size(); ++i) {
    segments_.push_back(RouteSegment{common::Segment{
      points_[i].point,
      points_[i + 1].point
    }});
  }
}

RoutePoint Route::GetPointFromStart(double len) {
  double pref = 0;
  for (size_t i = 0; i < segments_.size(); ++i) {
    double dist = common::GetHaversineDistance(segments_[i].segment);
    if (len <= pref + dist) {
      auto vec = points_[i + 1].point - points_[i].point;
      double k = (len - pref) / dist;
      return RoutePoint{
        segments_[i].segment.Start + vec * k,
        i,
        len,
        len - pref
      };
    }
    pref += dist;
  }

  return points_.back();
}

}  // namespace marine_navi::entities
