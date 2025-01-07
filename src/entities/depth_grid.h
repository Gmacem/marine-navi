#pragma once

#include <filesystem>
#include <fstream>
#include <optional>
#include <vector>

#include "common/geom.h"
#include "common/utils.h"
#include "entities/depth_point.h"

namespace marine_navi::entities {

class DepthGrid {
public:
  DepthGrid(std::string filepath);

  std::optional<double> GetDepth(const common::Point point) const;
  std::optional<common::Point> GetNearest(const common::Point point) const;
  std::optional<entities::DepthPoint> GetNearestDepthPoint(const common::Point point) const;

  std::vector<entities::DepthPoint> GetAllPoints() const;

private:
  uint32_t nCols_;
  uint32_t nRows_;
  double xCorner_;
  double yCorner_;
  double cellSize_;
  int noDataValue_;
  std::vector<std::vector<int> > data_;

  double minLat_;
  double maxLat_;
  double minLon_;
  double maxLon_;
};

}  // namespace marine_navi::entities
