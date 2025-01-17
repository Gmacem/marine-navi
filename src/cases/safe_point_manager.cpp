#include "safe_point_manager.h"

namespace marine_navi::cases {

SafePointManager::SafePointManager(std::shared_ptr<clients::DbClient> db_client): db_client_(db_client) {}


void SafePointManager::Load(const entities::SafePoint& safe_point) {
    db_client_->InsertSafePoints({safe_point});
}

std::vector<entities::SafePoint> SafePointManager::GetSafePoints() {
    return db_client_->SelectSafePoints();
}

} // namespace marine_navi::cases
