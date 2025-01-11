#pragma once

#include <memory>

class wxWindow;

namespace SQLite {
class Database;
}

namespace marine_navi {
namespace cases {
class MarineRouteScanner;
class ForecastsLoader;
class DepthLoader;
} // namespace cases

namespace clients {
class DbClient;
namespace query_builder {
class SqlQueryStorage;
} // namespace query_builder
} // namespace clients

class RenderOverlay;

struct Dependencies {
  std::shared_ptr<cases::MarineRouteScanner> marine_route_scanner;
  std::shared_ptr<cases::ForecastsLoader> forecasts_loader;
  std::shared_ptr<cases::DepthLoader> depth_loader;
  std::shared_ptr<marine_navi::RenderOverlay> render_overlay;
  std::shared_ptr<SQLite::Database> db;
  std::shared_ptr<clients::query_builder::SqlQueryStorage> sql_query_storage;
  std::shared_ptr<clients::DbClient> db_client;
  wxWindow* ocpn_canvas_window;
};

Dependencies CreateDependencies(wxWindow* ocpn_canvas_window);

}  // namespace marine_navi