#include <compute/instance.hpp>
#include <compute/device.hpp>
#include <compute/pipeline.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <memory>

int main() {
    auto logger = spdlog::stdout_color_mt("logger");

    spdlog::set_level(spdlog::level::trace);
    spdlog::trace("logger initialized");

    try {
        auto instance = std::make_shared<Compute::Instance>();
        auto device = std::make_shared<Compute::Device>(instance);
        auto pipeline = std::make_shared<Compute::Pipeline>(device);
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