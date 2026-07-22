#pragma once

#include <plaquette/workload/workload.hpp>

namespace Plaq::Workload {
    /// @brief Generates a large amount of random numbers on the GPU using the given seed,
    /// testing the Xoshiro256+ generator.
    void randomWorkload(const WorkloadInfo& info, uint64_t seed);
}