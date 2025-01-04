#pragma once

#include <vector>

#include <ocpn_plugin.h>

#include "common/utils.h"

namespace marine_navi::entities {

class Route {
public:
  Route(std::vector<PlugIn_Waypoint> route) : route_(route) {
    for (size_t i = 0; i < route_.size(); ++i) {
      points_.push_back(Utils::Point{route_[i].m_lat, route_[i].m_lon});
    }
  }

  double GetDistance() const;
  Utils::Point GetPointFromStart(double len);
  const std::vector<PlugIn_Waypoint>& GetRoute() const { return route_; }
  const std::vector<Utils::Point>& GetPoints() const { return points_; }

private:
  std::vector<Utils::Point> points_;
  std::vector<PlugIn_Waypoint> route_;
};

}  // namespace marine_navi::entities