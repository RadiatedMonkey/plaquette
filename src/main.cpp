#include <compute/instance.hpp>
#include <compute/device.hpp>
#include <compute/pipeline.hpp>
#include <compute/storage.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <vector>
#include <memory>

static const std::vector<float> firstData = {1, 2, 3};
static const std::vector<float> secondData = {3, 2, 1};

int main() {
    auto logger = spdlog::stdout_color_mt("logger");

    spdlog::set_level(spdlog::level::trace);
    spdlog::trace("logger initialized");

    try {
        auto instance = Compute::Instance::create();
        auto device = instance->createDevice();

        auto firstBuffer = std::make_shared<Compute::StorageBuffer>(device, firstData);
        auto secondBuffer = std::make_shared<Compute::StorageBuffer>(device, secondData);
        auto resultBuffer = std::make_shared<Compute::StorageBuffer>(device, std::size(secondData) * sizeof(float));

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