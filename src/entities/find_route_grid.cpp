#include "find_route_grid.h"

#include <algorithm>
#include <map>
#include <stdexcept>
#include <optional>

#include "common/marine_math.h"

namespace marine_navi::entities {

namespace {
struct IntPoint {
  int64_t x;
  int64_t y;
};

bool IsInsidePolygon(const std::vector<IntPoint>& polygon, const IntPoint& p) {
  int n = (int)polygon.size();
  bool inside = false;
  for (int i = 0, j = n - 1; i < n; j = i++) {
    if (polygon[i].x == p.x && polygon[i].y == p.y) {
      return true;
    }
    if (polygon[i].y == polygon[j].y && polygon[i].y == p.y &&
        std::min(polygon[i].x, polygon[j].x) <= p.x &&
        std::max(polygon[i].x, polygon[j].x) >= p.x) {
      return true;
    }

    bool intersectsY = ((polygon[i].y > p.y) != (polygon[j].y > p.y));
    if (intersectsY) {
      double xIntersect = static_cast<double>(polygon[j].x - polygon[i].x) *
                              (p.y - polygon[i].y) /
                              (polygon[j].y - polygon[i].y) +
                          polygon[i].x;
      if (xIntersect > p.x) {
        inside = !inside;
      }
    }
  }

  return inside;
}
}  // namespace

FindRouteGrid::FindRouteGrid(const common::Polygon& polygon, double step) {
  const int64_t kMaxCheckCount = 1000000;
  const int64_t kMaxVertexCount = 10000;
  int64_t minX = std::numeric_limits<int64_t>::max(),
          maxX = std::numeric_limits<int64_t>::min();
  int64_t minY = std::numeric_limits<int64_t>::max(),
          maxY = std::numeric_limits<int64_t>::min();

  std::vector<IntPoint> int_polygon;

  for (const auto& point : polygon.Points) {
    IntPoint pt{static_cast<int64_t>(point.X() / step), static_cast<int64_t>(point.Y() / step)};
    minX = std::min(minX, pt.x);
    maxX = std::max(maxX, pt.x);
    minY = std::min(minY, pt.y);
    maxY = std::max(maxY, pt.y);
    int_polygon.push_back(pt);
  }

  std::map<std::pair<int64_t, int64_t>, int> points_with_id;

  if (maxX - minX + 1 > kMaxCheckCount || maxY - minY + 1 > kMaxCheckCount ||
      (maxX - minX + 1) * (maxY - minY + 1) > kMaxVertexCount) {
    throw std::runtime_error("number points for check is too big");
  }

  for (int64_t x = minX; x <= maxX; x++) {
    for (int64_t y = minY; y <= maxY; y++) {
      IntPoint candidate = {x, y};
      if (IsInsidePolygon(int_polygon, candidate)) {
        points_with_id[{x, y}] = points_.size();
        points_.push_back(common::Point{y * step, x * step});
      }
    }
  }
  adjacency_list_.resize(points_.size());

  for (const auto& [point, id] : points_with_id) {
    for (int dx = -1; dx <= 1; dx++) {
      for (int dy = -1; dy <= 1; dy++) {
        if (dx == 0 && dy == 0) {
          continue;
        }
        if (points_with_id.find({point.first + dx, point.second + dy}) !=
            points_with_id.end()) {
          adjacency_list_[id].push_back(
              points_with_id[{point.first + dx, point.second + dy}]);
        }
      }
    }
    if (points_.size() > kMaxVertexCount) {
      throw std::runtime_error("number points is too big");
    }
  }

  if (points_.size() == 0) {
    throw std::runtime_error("no points");
  }
}

std::vector<common::Point> FindRouteGrid::GetAdjencyPoints(int point_id) const {
  std::vector<common::Point> result;
  for(const auto& id : adjacency_list_.at(point_id)) {
    result.push_back(points_.at(id));
  }
  return result;
}

int FindRouteGrid::GetClosestPointId(common::Point point) const {
  double min_distance = common::GetHaversineDistance(point, points_[0]);
  int result = 0;
  for(size_t i = 1; i < points_.size(); ++i) {
    double distance = common::GetHaversineDistance(point, points_[i]);
    if (distance < min_distance) {
      min_distance = distance;
      result = i;
    }
  }
  return result;
}

}  // namespace marine_navi::entities
