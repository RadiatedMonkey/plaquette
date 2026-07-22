#pragma once

#include <cstdint>

namespace Plaq::Workload {
    struct WorkloadInfo;

    void computeUniformMean(const WorkloadInfo& info, uint64_t seed);
}