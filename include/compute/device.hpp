#pragma once

#include <memory>

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#include <compute/command.hpp>

namespace Compute {
    class Instance;
    class StorageBuffer;
    template<typename T> class HostBuffer;

    class Device : public std::enable_shared_from_this<Device> {
    public:
        /// @warning The destructor should only run once all device-created resources have been destroyed.
        ~Device();

        /// @brief The raw Vulkan handle to this device.
        VkDevice handle();

        /// @brief The device queue.
        VkQueue queue();

        /// @brief Creates a host-visible, host-coherent buffer.
        /// @param size The size in elements of the buffer to create.
        ///
        /// @note This can be used as a staging buffer to populate device local buffers.
        template<typename T>
        std::shared_ptr<HostBuffer<T>> createHostBuffer(uint64_t size, VkBufferUsageFlags usageFlags);

        /// @brief Creates a device-local storage buffer of specified size.
        /// @param size The amount of elements of type `T` to fit into this buffer.
        ///
        /// @note This buffer cannot be mapped to host memory directly and must be populated by
        /// transferring from a staging buffer.
        template<typename T>
        std::shared_ptr<StorageBuffer> createStorageBuffer(uint64_t size, VkBufferUsageFlags usageFlags);

        /// @brief The raw Vulkan command pool handle.
        VkCommandPool cmdPool();

        /// @brief Creates a command buffer that can be recorded into.
        ///
        /// @warning The command buffer must be begun by the caller using `vkBeginCommandBuffer`.
        Commands createCmdBuffer();

        /// @brief The properties of this device.
        VkPhysicalDeviceProperties2 properties();

        /// @brief The features of this device.
        VkPhysicalDeviceFeatures2 features();

        /// @brief The memory properties of this device.
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