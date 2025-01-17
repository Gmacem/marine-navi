#pragma once

#include "common/geom.h"

namespace marine_navi::cases::scorers {

class IScorer {
public:
    virtual int64_t GetScore(int start_id, int end_id, time_t depart_time) = 0;
    virtual time_t GetArrivalTime(int start_id, int end_id, time_t depart_time) = 0;

    virtual ~IScorer() = default;

    static constexpr int64_t kMaxScore = 1e12;
};

} // namespace marine_navi::cases
