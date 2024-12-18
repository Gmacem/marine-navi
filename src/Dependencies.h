#pragma once

#include <memory>

class wxWindow;

namespace SQLite {
class Database;
}

namespace marine_navi {
namespace cases {
class CheckPathCase;
class ForecastsLoader;
} // namespace cases

namespace clients {
class DbClient;
namespace query_builder {
class SqlQueryStorage;
} // namespace query_builder
} // namespace clients

class RenderOverlay;

struct Dependencies {
  std::shared_ptr<cases::CheckPathCase> CheckPathCase;
  std::shared_ptr<cases::ForecastsLoader> ForecastsLoader;
  std::shared_ptr<marine_navi::RenderOverlay> RenderOverlay;
  std::shared_ptr<SQLite::Database> Db;
  std::shared_ptr<clients::query_builder::SqlQueryStorage> SqlQueryStorage;
  std::shared_ptr<clients::DbClient> DbClient;
  wxWindow* OcpnCanvasWindow;
};

Dependencies CreateDependencies(wxWindow* OcpnCanvasWindow);

}  // namespace marine_navi