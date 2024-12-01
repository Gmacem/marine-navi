#include "Dependencies.h"

#include "cases/CheckPathCase.h"
#include "cases/ForecastsLoader.h"
#include "clients/DbClient.h"
#include "clients/Esimo.h"
#include "RenderOverlay.h"

namespace MarineNavi {

Dependencies CreateDependencies(wxWindow* ocpnCanvasWindow) {
  Dependencies deps;
  deps.OcpnCanvasWindow = ocpnCanvasWindow;
  deps.Db = CreateDatabase("marinenavi.db");
  deps.DbClient = std::make_shared<DbClient>(deps.Db);
  deps.ForecastsLoader =
      std::make_shared<MarineNavi::cases::ForecastsLoader>(deps.DbClient);
  deps.CheckPathCase =
      std::make_shared<MarineNavi::cases::CheckPathCase>(deps.DbClient);
  deps.RenderOverlay = std::make_shared<MarineNavi::RenderOverlay>(deps);
  return deps;
}

}  // namespace MarineNavi
