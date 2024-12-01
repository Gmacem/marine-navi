#include "CheckPathCase.h"

namespace MarineNavi::cases {

namespace {}  // namespace

CheckPathCase::CheckPathCase(std::shared_ptr<MarineNavi::DbClient> dbClient)
    : mutex_(), pathData_(), show_(false), dbClient_(dbClient) {}

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

std::optional<entities::Diagnostic> CheckPathCase::GetDiagnostic() {
  std::lock_guard lock(mutex_);
  return diagnostic_;
}

void CheckPathCase::CrossDetect() {
  std::lock_guard lock(mutex_);
  fprintf(stderr, "Cross detect\n");

  try {
    diagnostic_ = DoCrossDetect();
  } catch (std::exception& ex) {
    fprintf(stderr, "Failed detect impl %s\n", ex.what());
    diagnostic_ = std::nullopt;  // TODO: signal about it
  }
}

std::optional<entities::Diagnostic> CheckPathCase::DoCrossDetect() const {
  static constexpr int ITER_NUM = 50;
  std::optional<DepthGrid> grid;
  auto route = pathData_.Route;
  if (pathData_.PathToDepthFile.has_value() &&
      pathData_.ShipDraft.has_value()) {
    grid = DepthGrid(pathData_.PathToDepthFile.value());
  }
  std::vector<std::pair<int, Point> > pathPoints;

  double total = route->GetDistance();

  for (int i = 0; i <= ITER_NUM; ++i) {
    double k = static_cast<double>(i) / ITER_NUM;
    pathPoints.emplace_back(i, route->GetPointFromStart(k * total));
  }

  auto forecasts = dbClient_->SelectNearestForecasts(
      pathPoints, Utils::CurrentFormattedTime());

  std::unordered_map<int, double> forecast_by_point;
  std::unordered_map<int, int> forecastIdByPoint;
  for (auto& [forecastId, id, wave_height, swell_height] : forecasts) {
    if (!wave_height.has_value()) {
      continue;
    }
    double height = wave_height.value();
    height += swell_height.has_value() ? swell_height.value() : 0;
    forecast_by_point[id] = height;
    forecastIdByPoint[id] = forecastId;
  }

  for (size_t i = 0; i < pathPoints.size(); ++i) {
    auto& p = pathPoints[i].second;
    int id = pathPoints[i].first;

    auto it = forecast_by_point.find(id);

    printf("Point: %u %f %f %d %d\n", i, p.Lat, p.Lon, grid.has_value(),
           it != forecast_by_point.end());

    if (pathData_.MaxWaveHeight.has_value() && it != forecast_by_point.end()) {
      if (it->second >= pathData_.MaxWaveHeight) {
        printf("Max wave dected\n");
        return entities::CreateHighWavesDiagnostic(
            p, dbClient_->GetForecastLocation(forecastIdByPoint[id]),
            it->second, "esimo.ru", std::time(0));
      }
    }

    if (grid.has_value() &&
        !CheckDepth(grid.value(), p, pathData_.ShipDraft.value())) {
      printf("Max depth dected\n");
      return entities::CreateNotDeepDiagnostic(
          p, grid->GetNearest(p.Lat, p.Lon).value(),
          grid->GetDepth(p.Lat, p.Lon).value(), "gebco.net", std::time(0));
    }
  }
  printf("Cross not detected\n");

  return std::nullopt;
}

}  // namespace MarineNavi::cases
