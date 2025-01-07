#pragma once

#include <vector>

#include <ocpn_plugin.h>

#include "common/geom.h"
#include "common/utils.h"

namespace marine_navi::entities {

struct RoutePoint {
  common::Point point;
  size_t segment_id;
  double distance_from_start_route;
  double distance_from_start_segment;
};

struct RouteSegment {
  common::Segment segment;
};

class Route {
public:
  Route(std::vector<PlugIn_Waypoint> waypoints);

  double GetDistance() const { return total_distance_; }
  RoutePoint GetPointFromStart(double len);
  const std::vector<RoutePoint>& GetPoints() const { return points_; }
  const std::vector<RouteSegment>& GetSegments() const { return segments_; }

private:
  const std::vector<PlugIn_Waypoint> waypoints_;
  std::vector<RoutePoint> points_;
  std::vector<RouteSegment> segments_;
  double total_distance_;
};

}  // namespace marine_navi::entities