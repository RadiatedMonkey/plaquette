#pragma once

#include <vector>
#include <memory>

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

namespace Compute {
    class Device;

    class StorageBuffer {
    public:
        StorageBuffer(std::shared_ptr<Device> device, VkDeviceSize size);
        StorageBuffer(std::shared_ptr<Device> device, const std::vector<float>& data);

        ~StorageBuffer();

    private:
        void destroyBuffer();

        std::shared_ptr<Device> mDevice = nullptr;

        VkDeviceSize mSize = 0;

        VkDeviceMemory mMemory = VK_NULL_HANDLE;
        VkBuffer mBuffer = VK_NULL_HANDLE;
    };

    class UniformBuffer {
    public:
        UniformBuffer(std::shared_ptr<Device> device);
        ~UniformBuffer();

    private:
        std::shared_ptr<Device> mDevice = nullptr;
    };
}