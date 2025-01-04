#include "depth_grid.h"

namespace marine_navi::entities {

namespace {
template <typename T>
void ReadAndCheck(std::ifstream& file, std::string expectedKey, T& value) {
  std::string key;
  file >> key >> value;
  if (key != expectedKey) {
    throw std::runtime_error("unexpected key: " + key);
  }
}
}  // namespace

DepthGrid::DepthGrid(std::string filepath) {
  std::ifstream file(filepath);

  ReadAndCheck(file, "ncols", nCols_);
  ReadAndCheck(file, "nrows", nRows_);
  ReadAndCheck(file, "xllcorner", xCorner_);
  ReadAndCheck(file, "yllcorner", yCorner_);
  ReadAndCheck(file, "cellsize", cellSize_);
  ReadAndCheck(file, "NODATA_value", noDataValue_);

  data_.resize(nRows_, std::vector<int>(nCols_, noDataValue_));
  for (size_t i = 0; i < nRows_; ++i) {
    for (size_t j = 0; j < nCols_; ++j) {
      file >> data_[i][j];
    }
  }

  minLat_ = yCorner_;
  maxLat_ = yCorner_ + cellSize_ * nRows_;

  minLon_ = xCorner_;
  maxLon_ = xCorner_ + cellSize_ * nCols_;
}

std::optional<double> DepthGrid::GetDepth(double lat, double lon) const {
  if (maxLon_ > 180 && lon < 0) {
    lon = 360 + lon;
  }
  if (minLon_ > lon || lon >= maxLon_ || minLat_ > lat || lat >= maxLat_) {
    return std::nullopt;
  }
  int r = (lat - minLat_) / cellSize_;
  int c = (lon - minLon_) / cellSize_;

  printf("Get depth: %f %f %f %f %d %d %d %d\n", lat, lon, minLat_, minLon_, r,
         c, nRows_, nCols_);

  if (r < 0 || r >= nRows_ || c < 0 || c >= nCols_) {
    return std::nullopt;
  }
  return data_[data_.size() - r - 1][c];
}

std::optional<Utils::Point> DepthGrid::GetNearest(double lat,
                                                  double lon) const {
  if (maxLon_ > 180 && lon < 0) {
    lon = 360 + lon;
  }
  if (minLon_ > lon || lon >= maxLon_ || minLat_ > lat || lat >= maxLat_) {
    return std::nullopt;
  }
  int r = (lat - minLat_) / cellSize_;
  int c = (lon - minLon_) / cellSize_;
  if (r < 0 || r >= nRows_ || c < 0 || c >= nCols_) {
    return std::nullopt;
  }
  return Utils::Point{r * cellSize_ + minLat_, c * cellSize_ + minLon_};
}

}  // namespace marine_navi::entities
