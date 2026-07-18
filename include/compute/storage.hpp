#pragma once

#include <compute/pipeline.hpp>
#include <compute/reflection.hpp>

#include <vector>
#include <memory>

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

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

            auto stagingBuffer = createBoundBuffer(mSize, false);

            
        }

        ~StorageBuffer();

    private:
        BufferReference createBoundBuffer(VkDeviceSize size, bool deviceLocal);
        void destroyBuffer();

        std::shared_ptr<Device> mDevice = nullptr;

        VkDeviceSize mSize = 0;
        VkBuffer mBuffer = VK_NULL_HANDLE;
        VkDeviceMemory mMemory = VK_NULL_HANDLE;
    };
}