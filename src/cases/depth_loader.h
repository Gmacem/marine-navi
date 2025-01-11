#pragma once

#include <memory>
#include <string>

#include <clients/db_client.h>

namespace marine_navi::cases {

class DepthLoader {
public:
    DepthLoader(std::shared_ptr<clients::DbClient> db_client): db_client_(db_client) {}
    void Load(const std::string& path);

private:
  std::shared_ptr<clients::DbClient> db_client_;

};

} // namespace marine_navi::cases