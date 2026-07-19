#include <compute/instance.hpp>
#include <compute/device.hpp>
#include <compute/pipeline.hpp>
#include <compute/storage.hpp>
#include <compute/log.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <vector>
#include <memory>

static const std::vector<float> firstData = {1, 2, 3};
static const std::vector<float> secondData = {3, 2, 1};

int main() {
    auto logger = spdlog::stdout_color_mt("logger");

    spdlog::set_level(spdlog::level::trace);
    spdlog::trace("Logger initialized");

    try {
        auto instance = Compute::Instance::create();
        auto device = instance->createDevice();

        auto cmds = device->createCmdBuffer();

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        LOG_VKRESULT(
            vkBeginCommandBuffer(cmds.handle(), &beginInfo),
            "Failed to start command buffer"
        );

        auto staging = device->createHostBuffer<float>(firstData.size());
        {
            auto mapped = staging->map();
            std::memcpy(mapped.get(), firstData.data(), firstData.size() * sizeof(float));
        }

        auto staging2 = device->createHostBuffer<float>(secondData.size());
        {
            auto mapped = staging2->map();
            std::memcpy(mapped.get(), secondData.data(), secondData.size() * sizeof(float));
        }

        auto firstBuffer = device->createStorageBuffer<float>(firstData.size());
        auto secondBuffer = device->createStorageBuffer<float>(secondData.size());
        auto resultBuffer = device->createStorageBuffer<float>(secondData.size());

        VkBufferCopy2 copyRegion = {};
        copyRegion.sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2;
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = staging->size();

        VkCopyBufferInfo2 copyInfo = {};
        copyInfo.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2;
        copyInfo.srcBuffer = staging->handle();
        copyInfo.dstBuffer = firstBuffer->handle();
        copyInfo.regionCount = 1;
        copyInfo.pRegions = &copyRegion;

        vkCmdCopyBuffer2(cmds.handle(), &copyInfo);

        copyInfo.srcBuffer = staging2->handle();
        copyInfo.dstBuffer = secondBuffer->handle();

        vkCmdCopyBuffer2(cmds.handle(), &copyInfo);

        LOG_VKRESULT(
            vkEndCommandBuffer(cmds.handle()),
            "Failed to end command buffer"
        );

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