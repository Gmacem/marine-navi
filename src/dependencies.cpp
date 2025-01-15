#include "dependencies.h"

#include <wx/log.h>

#include "cases/best_route_maker.h"
#include "cases/depth_loader.h"
#include "cases/forecasts_loader.h"
#include "cases/marine_route_scanner.h"
#include "cases/safe_point_manager.h"
#include "clients/db_client.h"
#include "clients/query_builder/sql_query_storage.h"
#include "clients/esimo.h"
#include "render_overlay.h"

namespace marine_navi {

namespace {
std::shared_ptr<clients::SqlQueryStorage> MakeSqlQueryStorage() {
  wxFileName fn;
  wxString tmp_path;
  tmp_path = GetPluginDataDir("MarineNavi_pi");
  fn.SetPath(tmp_path);
  fn.AppendDir("data");
  fn.AppendDir("queries");
  wxLogInfo(wxT("Path to queries '%s'"), fn.GetPath().ToStdString());

  return std::make_shared<clients::SqlQueryStorage>(fn.GetPath().ToStdString());
}
} // namespace 

Dependencies CreateDependencies(wxWindow* ocpnCanvasWindow) {
  Dependencies deps;

  deps.ocpn_canvas_window = ocpnCanvasWindow;
  deps.sql_query_storage = MakeSqlQueryStorage();
  deps.db = clients::CreateDatabase("marinenavi.db", deps.sql_query_storage);
  deps.db_client = std::make_shared<clients::DbClient>(deps.db, deps.sql_query_storage);
  deps.depth_loader = std::make_shared<cases::DepthLoader>(deps.db_client);
  deps.best_route_maker = std::make_shared<cases::BestRouteMaker>(deps.db_client);
  deps.forecasts_loader =
      std::make_shared<cases::ForecastsLoader>(deps.db_client);
  deps.marine_route_scanner =
      std::make_shared<cases::MarineRouteScanner>(deps.db_client);
  deps.safe_point_manager = std::make_shared<cases::SafePointManager>(deps.db_client);
  deps.render_overlay = std::make_shared<marine_navi::RenderOverlay>(deps);
  return deps;
}

}  // namespace marine_navi
