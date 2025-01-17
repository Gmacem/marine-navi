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

std::optional<double> DepthGrid::GetDepth(const common::Point point) const {
  const auto depth_point = GetNearestDepthPoint(point);
  if (depth_point.has_value()) {
    return depth_point.value().Depth;
  }
  return std::nullopt;
}

std::optional<common::Point> DepthGrid::GetNearest(const common::Point point) const {
  const auto depth_point = GetNearestDepthPoint(point);
  if (depth_point.has_value()) {
    return depth_point.value().Point;
  }
  return std::nullopt;
}

std::optional<entities::DepthPoint> DepthGrid::GetNearestDepthPoint(const common::Point point) const {
  double lat = point.Lat;
  double lon = point.Lon;
  if (maxLon_ > 180 && lon < 0) {
    lon = 360 + lon;
  }
  if (minLon_ > lon || lon >= maxLon_ || minLat_ > lat || lat >= maxLat_) {
    return std::nullopt;
  }
  size_t r = (lat - minLat_) / cellSize_;
  size_t c = (lon - minLon_) / cellSize_;
  if (r < 0 || r >= nRows_ || c < 0 || c >= nCols_) {
    return std::nullopt;
  }
  return entities::DepthPoint{
    common::Point{r * cellSize_ + minLat_, c * cellSize_ + minLon_},
    static_cast<double>(data_[data_.size() - r - 1][c])
  };
}

std::vector<entities::DepthPoint> DepthGrid::GetAllPoints() const {
  std::vector<entities::DepthPoint> result;

  for(size_t i = 0; i < data_.size(); ++i) {
    for(size_t j = 0; j < data_[i].size(); ++j) {
      result.push_back(
        entities::DepthPoint{
          common::Point{
            i * cellSize_ + minLat_,
            j * cellSize_ + minLon_
          },
          static_cast<double>(data_[data_.size() - i - 1][j])
      });
    }
  }

  return result;
}

}  // namespace marine_navi::entities
