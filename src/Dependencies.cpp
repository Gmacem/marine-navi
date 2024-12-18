#include "Dependencies.h"

#include <wx/log.h>

#include <cases/CheckPathCase.h>
#include <cases/ForecastsLoader.h>
#include <clients/DbClient.h>
#include <clients/query_builder/SqlQueryStorage.h>
#include <clients/Esimo.h>
#include <RenderOverlay.h>

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

  deps.OcpnCanvasWindow = ocpnCanvasWindow;
  deps.SqlQueryStorage = MakeSqlQueryStorage();
  deps.Db = clients::CreateDatabase("marinenavi.db", deps.SqlQueryStorage);
  deps.DbClient = std::make_shared<clients::DbClient>(deps.Db);
  deps.ForecastsLoader =
      std::make_shared<cases::ForecastsLoader>(deps.DbClient);
  deps.CheckPathCase =
      std::make_shared<cases::CheckPathCase>(deps.DbClient);
  deps.RenderOverlay = std::make_shared<marine_navi::RenderOverlay>(deps);
  return deps;
}

}  // namespace marine_navi
