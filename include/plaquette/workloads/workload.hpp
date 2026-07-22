#pragma once

#include <memory>

namespace Plaq {
    class Device;
}

namespace Plaq::Workload {
    struct WorkloadInfo {
        std::shared_ptr<Device> device;
    };
}