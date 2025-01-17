#pragma once

#include <vector>

#include "common/geom.h"

namespace marine_navi::entities {

class FindRouteGrid {
public:
    FindRouteGrid(const common::Polygon& polygon, double step);

    const std::vector<common::Point>& GetPoints() const { return points_; }
    common::Point GetPoint(size_t id) const { return points_.at(id); }
    std::vector<common::Point> GetAdjencyPoints(int point_id) const;
    const std::vector<int>& GetAdjencyPointIds(int point_id) const { return adjacency_list_.at(point_id); }
    int GetClosestPointId(common::Point point) const;

private:
    std::vector<std::vector<int> > adjacency_list_;
    std::vector<common::Point> points_;
};

} // namespace marine_navi::entities
