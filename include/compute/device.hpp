#pragma once

#include <memory>

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#include <compute/command.hpp>

namespace Compute {
    class Instance;

    class Device : public std::enable_shared_from_this<Device> {
    public:
        ~Device();

        VkDevice handle();

        VkFence submit(Commands commands);

        VkCommandPool pool();
        Commands record();

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