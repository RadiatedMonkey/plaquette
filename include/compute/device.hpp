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

    private:
        std::shared_ptr<Instance> mInstance = nullptr;

        VkDevice mDevice = VK_NULL_HANDLE;
    };
}