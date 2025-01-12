#pragma once

#include <memory>

#include "clients/db_client.h"

namespace marine_navi::cases {

class SafePointManager {
public:
  SafePointManager(std::shared_ptr<clients::DbClient> db_client);

  void Load(const entities::SafePoint& safe_point);
  std::vector<entities::SafePoint> GetSafePoints();

private:
  std::shared_ptr<clients::DbClient> db_client_;

};

}  // namespace marine_navi::cases