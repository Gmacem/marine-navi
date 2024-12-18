#include "CheckPathCase.h"

namespace marine_navi::cases {

CheckPathCase::CheckPathCase(std::shared_ptr<marine_navi::clients::DbClient> db_client)
    : mutex_(), pathData_(), show_(false), db_client_(db_client) {}

void CheckPathCase::SetPathData(const PathData& pathData) {
  std::lock_guard lock(mutex_);
  pathData_ = pathData;
}

const PathData& CheckPathCase::GetPathData() {
  std::lock_guard lock(mutex_);
  return pathData_;
}

void CheckPathCase::SetShow(bool show) {
  std::lock_guard lock(mutex_);
  show_ = show;
}

bool CheckPathCase::IsShow() {
  std::lock_guard lock(mutex_);
  return show_;
}

bool CheckPathCase::CheckLandIntersection(const Point& p1,
                                          const Point& p2) const {
  static constexpr double EPS = 1e-5;

  Point minCorner{std::min(p1.Lat, p2.Lat), std::min(p1.Lon, p2.Lon)};
  Point maxCorner{std::max(p1.Lat, p2.Lat), std::max(p1.Lon, p2.Lon)};
  minCorner.Lat -= EPS;
  minCorner.Lon -= EPS;
  maxCorner.Lat += EPS;
  maxCorner.Lon += EPS;

  return PlugIn_GSHHS_CrossesLand(minCorner.Lat, minCorner.Lon, maxCorner.Lat,
                                  maxCorner.Lon);
}

bool CheckPathCase::CheckDepth(const DepthGrid& grid, const Point& p,
                               double draft) const {
  auto depth = grid.GetDepth(p.Lat, p.Lon);

  if (depth.has_value()) {
    return -depth.value() > draft;  // TODO perhaps it needs to be taken into
                                    // account with some margin
  }

  return true;
}

std::optional<wxPoint2DDouble> CheckPathCase::GetLastResult() {
  std::lock_guard lock(mutex_);
  return lastResult_;
}

void CheckPathCase::CrossDetect() {
  std::lock_guard lock(mutex_);
  fprintf(stderr, "Cross detect\n");

  try {
    lastResult_ = CrossDetectImpl();
  } catch (std::exception& ex) {
    fprintf(stderr, "Failed detect impl %s\n", ex.what());
    lastResult_ = std::nullopt;
  }
}

std::optional<wxPoint2DDouble> CheckPathCase::CrossDetectImpl() const {
  static constexpr int ITER_NUM = 50;
  const Point start = pathData_.Start;
  const Point end = pathData_.End;
  std::optional<DepthGrid> grid;
  if (pathData_.PathToDepthFile.has_value() &&
      pathData_.ShipDraft.has_value()) {
    grid = DepthGrid(pathData_.PathToDepthFile.value());
  }

  Point vec = end - start;

  std::vector<std::pair<int, Point> > pathPoints;

  for (int i = 0; i <= ITER_NUM; ++i) {
    double k = static_cast<double>(i) / ITER_NUM;
    Point p = start + vec * k;
    pathPoints.emplace_back(i, p);
  }

  auto forecasts = db_client_->SelectNearestForecasts(
      pathPoints, Utils::CurrentFormattedTime());

  std::unordered_map<int, double> forecast_by_point;
  for (auto& [id, wave_height, swell_height] : forecasts) {
    if (!wave_height.has_value()) {
      continue;
    }
    double height = wave_height.value();
    height += swell_height.has_value() ? swell_height.value() : 0;
    forecast_by_point[id] = height;
  }

  for (size_t i = 0; i < pathPoints.size(); ++i) {
    auto& p = pathPoints[i].second;

    auto it = forecast_by_point.find(pathPoints[i].first);
    if (pathData_.MaxWaveHeight.has_value() && it != forecast_by_point.end()) {
      if (it->second >= pathData_.MaxWaveHeight) {
        return wxPoint2DDouble(p.Lat, p.Lon);
      }
    }

    if (grid.has_value() &&
        !CheckDepth(grid.value(), p, pathData_.ShipDraft.value())) {
      return wxPoint2DDouble(p.Lat, p.Lon);
    }

    if (CheckLandIntersection(pathData_.Start, p)) {
      return wxPoint2DDouble(p.Lat, p.Lon);
    }
  }
  return std::nullopt;
}

}  // namespace marine_navi::cases
