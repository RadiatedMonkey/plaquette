#include <compute/instance.hpp>
#include <compute/device.hpp>
#include <compute/pipeline.hpp>
#include <compute/storage.hpp>
#include <compute/log.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <iostream>
#include <vector>
#include <memory>

static constexpr uint32_t RNG_NUM_COUNT = 1024;

static const std::vector<float> firstData = {1, 2, 16};
static const std::vector<float> secondData = {3, 2, 1};

struct PushConstants {
    uint64_t baseSeed;
    uint32_t totalSites;
    uint32_t pad0;
    VkDeviceAddress output;
};

void printBuckets(const std::vector<float>& results) {
    static constexpr size_t BUCKET_COUNT = 10;

    std::array<uint32_t, BUCKET_COUNT> buckets = {};
    for(float num : results) {
        float ratio = static_cast<float>(BUCKET_COUNT) * num;
        uint32_t bucket = static_cast<uint32_t>(std::floor(ratio));

        buckets[bucket]++;
    }

    for(uint32_t i = 0; i < BUCKET_COUNT; i++) {
        uint32_t bucket = buckets[i];
        std::cout << i << ": (" << bucket << "): " << std::string(bucket, '*') << std::endl;
    }

    std::cout << "Expected average of " << static_cast<float>(RNG_NUM_COUNT) / static_cast<float>(BUCKET_COUNT) << " per bucket" << std::endl;
}

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

        auto outBuffer = device->createStorageBuffer<float>(RNG_NUM_COUNT, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
        auto hostOutBuffer = device->createHostBuffer<float>(RNG_NUM_COUNT, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

        PushConstants pushConstants = {
            .baseSeed = 0,
            .totalSites = RNG_NUM_COUNT,
            .output = outBuffer->address()
        };

        vkCmdBindPipeline(cmds.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->handle());
        vkCmdPushConstants(cmds.handle(), pipeline->layout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PushConstants), &pushConstants);
        vkCmdDispatch(cmds.handle(), (RNG_NUM_COUNT + 64 - 1) / 64, 1, 1);

        VkBufferMemoryBarrier2 resultBarrier = {};
        resultBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
        resultBarrier.buffer = outBuffer->handle();
        resultBarrier.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        resultBarrier.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;
        resultBarrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        resultBarrier.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
        resultBarrier.offset = 0;
        resultBarrier.size = VK_WHOLE_SIZE;

        VkDependencyInfo depInfo = {};
        depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        depInfo.bufferMemoryBarrierCount = 1;
        depInfo.pBufferMemoryBarriers = &resultBarrier;

        vkCmdPipelineBarrier2(cmds.handle(), &depInfo);

        VkBufferCopy2 copyRegion = {};
        copyRegion.sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2;
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = RNG_NUM_COUNT * sizeof(float);

        VkCopyBufferInfo2 copyInfo = {};
        copyInfo.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2;
        copyInfo.regionCount = 1;
        copyInfo.pRegions = &copyRegion;
        copyInfo.dstBuffer = hostOutBuffer->handle();
        copyInfo.srcBuffer = outBuffer->handle();

        vkCmdCopyBuffer2(cmds.handle(), &copyInfo);

        LOG_VKRESULT(
            vkEndCommandBuffer(cmds.handle()),
            "Failed to end command buffer"
        );

        VkCommandBufferSubmitInfo cmdInfo = {};
        cmdInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        cmdInfo.commandBuffer = cmds.handle();

        VkSubmitInfo2 submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submitInfo.commandBufferInfoCount = 1;
        submitInfo.pCommandBufferInfos = &cmdInfo;
        
        VkFenceCreateInfo fenceCi = {};
        fenceCi.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

        VkFence fence = VK_NULL_HANDLE;
        LOG_VKRESULT(
            vkCreateFence(device->handle(), &fenceCi, nullptr, &fence),
            "Failed to create fence"
        );

        CHECK_VKRESULT(
            vkQueueSubmit2(device->queue(), 1, &submitInfo, fence),
            "Failed to submit command buffer to queue",
            {
                vkDestroyFence(device->handle(), fence, nullptr);
                spdlog::debug("Destroyed fence");
            }
        );

        CHECK_VKRESULT(
            vkWaitForFences(device->handle(), 1, &fence, VK_TRUE, UINT64_MAX),
            "Failed to wait for queue fence",
            {
                vkDestroyFence(device->handle(), fence, nullptr);
                spdlog::debug("Destroyed fence");
            }
        );

        vkDestroyFence(device->handle(), fence, nullptr);
        spdlog::debug("Destroyed fence");

        auto mapped = hostOutBuffer->map();

        std::vector<float> results(RNG_NUM_COUNT);
        std::memcpy(results.data(), mapped.get(), RNG_NUM_COUNT * sizeof(float));

        // for (float result : results) {
        //     std::cout << result << " ";
        // }
        // std::cout << std::endl;

        printBuckets(results);
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