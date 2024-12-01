#pragma once

#include <memory>

class wxWindow;

namespace SQLite {
class Database;
}

namespace MarineNavi {
namespace cases {
class CheckPathCase;
class ForecastsLoader;
}  // namespace cases

class DbClient;
class RenderOverlay;

struct Dependencies {
  std::shared_ptr<MarineNavi::cases::CheckPathCase> CheckPathCase;
  std::shared_ptr<MarineNavi::cases::ForecastsLoader> ForecastsLoader;
  std::shared_ptr<MarineNavi::RenderOverlay> RenderOverlay;
  std::shared_ptr<SQLite::Database> Db;
  std::shared_ptr<MarineNavi::DbClient> DbClient;
  wxWindow* OcpnCanvasWindow;
};

Dependencies CreateDependencies(wxWindow* OcpnCanvasWindow);

}  // namespace MarineNavi