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

struct PushConstants {
    VkDeviceAddress buffer0;
    VkDeviceAddress buffer1;
    VkDeviceAddress result;
};

int main() {
    auto logger = spdlog::stdout_color_mt("logger");

    spdlog::set_level(spdlog::level::trace);
    spdlog::trace("Logger initialized");

    try {
        auto instance = Compute::Instance::create();
        auto device = instance->createDevice();
        auto pipeline = std::make_shared<Compute::Pipeline>(device);

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

        std::array<VkBuffer, 2> buffers = {
            firstBuffer->handle(), secondBuffer->handle()
        };
        std::array<VkBufferMemoryBarrier2, 2> memBarriers = {};

        for(uint32_t i = 0; i < memBarriers.size(); i++) {
            VkBufferMemoryBarrier2& memBarrier = memBarriers[i];
            memBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
            memBarrier.buffer = buffers[i];
            memBarrier.offset = 0;
            memBarrier.size = VK_WHOLE_SIZE;
            memBarrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
            memBarrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
            memBarrier.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
            memBarrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
        }

        VkDependencyInfo depInfo = {};
        depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        depInfo.bufferMemoryBarrierCount = memBarriers.size();
        depInfo.pBufferMemoryBarriers = memBarriers.data();

        vkCmdPipelineBarrier2(cmds.handle(), &depInfo);

        PushConstants pushConstants = {
            .buffer0 = firstBuffer->address(),
            .buffer1 = secondBuffer->address(),
            .result = resultBuffer->address()
        };

        vkCmdBindPipeline(cmds.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->handle());
        vkCmdPushConstants(cmds.handle(), pipeline->layout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PushConstants), &pushConstants);
        vkCmdDispatch(cmds.handle(), firstData.size(), 1, 1);

        LOG_VKRESULT(
            vkEndCommandBuffer(cmds.handle()),
            "Failed to end command buffer"
        );
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