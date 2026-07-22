#include <plaquette/instance.hpp>
#include <plaquette/device.hpp>
#include <plaquette/workloads/mean.hpp>
#include <plaquette/workloads/workload.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

int main() {
    auto logger = spdlog::stdout_color_mt("logger");

    spdlog::set_level(spdlog::level::trace);
    spdlog::trace("Logger initialized");

    try {
        auto instance = Plaq::Instance::create();
        auto device = instance->createDevice();

        Plaq::Workload::WorkloadInfo info = {
            .device = device
        };

        Plaq::Workload::computeUniformMean(info, 23497349734);
    } catch(const std::exception& e) {
        spdlog::error("exception: {}", e.what());
        return 1;
    } catch(...) {
        spdlog::error("unknown exception occurred");
        return 1;
    }

    spdlog::trace("exiting");

    return 0;
}