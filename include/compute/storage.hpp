#pragma once

#include <compute/pipeline.hpp>

#include <vector>
#include <memory>

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

namespace Compute {
    class Device;
    
    class StorageBuffer {
    public:
        /// @brief Allocate an uninitialized storage buffer.
        /// @param device The device to allocate on.
        /// @param size The size of the buffer to allocate. This must be greater than 0.
        StorageBuffer(std::shared_ptr<Device> device, VkDeviceSize size);

        /// @brief Allocates a storage buffer and initializes it with the given data.
        /// @param device The device to allocate on.
        /// @param data The data to copy to the buffer.
        StorageBuffer(std::shared_ptr<Device> device, const std::vector<float>& data);

        ~StorageBuffer();
        
        VkResult write(const std::vector<float>& data);

        VkBuffer handle() {
            return mBuffer;
        }

        VkDeviceMemory memory() {
            return mMemory;
        }

    private:
        /// @brief Destroys the buffer. This function does not free the associated memory.
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