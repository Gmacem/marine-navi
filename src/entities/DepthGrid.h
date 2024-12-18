#pragma once

#include <filesystem>
#include <fstream>
#include <optional>
#include <vector>

namespace marine_navi {

class DepthGrid {
public:
  DepthGrid(std::string filepath);

  std::optional<double> GetDepth(const double lat, double lon) const;

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

}  // namespace marine_navi
