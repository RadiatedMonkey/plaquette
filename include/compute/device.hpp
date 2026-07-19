#pragma once

#include <memory>

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#include <compute/storage.hpp>
#include <compute/command.hpp>

namespace Compute {
    class Instance;

    class Device : public std::enable_shared_from_this<Device> {
    public:
        ~Device();

        /// The raw Vulkan handle to this device.
        VkDevice handle();

        /// Creates a host-visible, host-coherent buffer with associated memory of size `size`.
        ///
        /// This can be used as a staging buffer to populate device local buffers.
        template<typename T>
        HostBuffer<T> createHostBuffer(VkDeviceSize size) {
            return HostBuffer<T>(shared_from_this(), size);
        }

        VkCommandPool cmdPool();
        Commands createCmdBuffer();

        VkPhysicalDeviceProperties2 properties();
        VkPhysicalDeviceFeatures2 features();
        VkPhysicalDeviceMemoryProperties2 memProperties();

    private:
        friend Instance;

        Device(std::shared_ptr<Instance> instance);

        void destroyDevice();

        std::shared_ptr<Instance> mInstance = nullptr;

        VkPhysicalDeviceMemoryProperties2 mMemProperties = {};
        VkPhysicalDeviceFeatures2 mFeatures = {};
        VkPhysicalDeviceProperties2 mProperties = {};

        VkCommandPool mPool = VK_NULL_HANDLE;
        VkQueue mQueue = VK_NULL_HANDLE;
        VkDevice mDevice = VK_NULL_HANDLE;

        uint32_t mQueueFamilyIndex = UINT32_MAX;
    };
}