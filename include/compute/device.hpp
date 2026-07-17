#pragma once

#include <memory>

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

namespace Compute {
    class Instance;

    class Device {
    public:
        Device(std::shared_ptr<Instance> instance);
        ~Device();

        VkDevice handle() {
            return mDevice;
        }

        VkPhysicalDeviceProperties properties() {
            return mProperties;
        }

        VkPhysicalDeviceFeatures features() {
            return mFeatures;
        }

        VkPhysicalDeviceMemoryProperties memProperties() {
            return mMemProperties;
        }

    private:
        std::shared_ptr<Instance> mInstance = nullptr;

        VkPhysicalDeviceMemoryProperties mMemProperties;
        VkPhysicalDeviceFeatures mFeatures;
        VkPhysicalDeviceProperties mProperties;
        VkDevice mDevice = VK_NULL_HANDLE;
    };
}