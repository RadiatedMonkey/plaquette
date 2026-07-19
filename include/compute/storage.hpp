#pragma once

#include <compute/pipeline.hpp>
#include <compute/reflection.hpp>
#include <compute/command.hpp>
#include <compute/device.hpp>

#include <vector>
#include <memory>

#include <spdlog/spdlog.h>
#include <volk.h>

namespace Compute {
    template<typename T>
    concept BasicBufferType = std::is_same_v<T, float> || std::is_same_v<T, double>;

    class Device;

    struct BufferReference {
        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
    };

    class StorageBuffer : public Resource {
    public:
        StorageBuffer(std::shared_ptr<Device> device, VkDeviceSize size);

        template<BasicBufferType T>
        StorageBuffer(std::shared_ptr<Device> device, std::vector<T> data) 
            : StorageBuffer(std::move(device), data.size() * sizeof(T))
        {
            // Device local buffer has already been created. Create host coherent staging buffer
            // and copy to device local buffer.

            auto stagingBuffer = createBoundBuffer(mSize, true);
            Commands cmds = mDevice->record();

            VkCommandBuffer cmdBuffer = cmds.handle();

            VkBufferCopy2 region = {};
            region.sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2;
            region.srcOffset = 0;
            region.dstOffset = 0;
            region.size = mSize;

            VkCopyBufferInfo2 copyInfo = {};
            copyInfo.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2;
            copyInfo.srcBuffer = stagingBuffer.buffer;
            copyInfo.dstBuffer = mBuffer;
            copyInfo.regionCount = 1;
            copyInfo.pRegions = &region;

            vkCmdCopyBuffer2(cmdBuffer, &copyInfo);

            mDevice->submit(std::move(cmds));

            spdlog::debug("Submitted transfer command for staging buffer");
        }

        ~StorageBuffer();

    private:
        BufferReference createBoundBuffer(VkDeviceSize size, bool isTransferSrc);

        void freeMemory();
        void destroyBuffer();

        std::shared_ptr<Device> mDevice = nullptr;

        VkDeviceSize mSize = 0;
        VkBuffer mBuffer = VK_NULL_HANDLE;
        VkDeviceMemory mMemory = VK_NULL_HANDLE;
    };
}