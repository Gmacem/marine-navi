#pragma once

#include <memory>

#include "entities/route.h"
#include "entities/ship.h"

namespace BestRouteMaker {

struct BestRouteInput {
  std::shared_ptr<entities::Route> Route;
  entities::ShipPerformanceInfo ShipPerformanceInfo;

  time_t DepartTime;
  std::optional<std::string> PathToDepthFile;
};

class BestRouteMaker{
public:
};


} // namespace BestRouteMaker
