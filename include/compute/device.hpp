#pragma once

#include <memory>

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#include <compute/command.hpp>

namespace Compute {
    class Instance;

    class Device : public std::enable_shared_from_this<Device> {
    public:
        Device(std::shared_ptr<Instance> instance);
        ~Device();

        VkDevice handle();
        Queue& queue();
        const Queue& queue() const;

        VkPhysicalDeviceProperties2 properties();
        VkPhysicalDeviceFeatures2 features();
        VkPhysicalDeviceMemoryProperties2 memProperties();

    private:
        std::shared_ptr<Instance> mInstance = nullptr;

        VkPhysicalDeviceMemoryProperties2 mMemProperties;
        VkPhysicalDeviceFeatures2 mFeatures;
        VkPhysicalDeviceProperties2 mProperties;

        Queue mQueue;
        VkDevice mDevice = VK_NULL_HANDLE;
    };
}