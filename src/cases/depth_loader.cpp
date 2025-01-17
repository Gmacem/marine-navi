#include "depth_loader.h"

#include <entities/depth_grid.h>

namespace marine_navi::cases {

void DepthLoader::Load(const std::string& path) {
    const auto grid = entities::DepthGrid(path);
    const auto points = grid.GetAllPoints();
    db_client_->InsertDepthPointBatch(points);
}

} // namespace marine_navi::cases
