#pragma once

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

namespace Compute {
    class Instance {
    public:
        Instance();
        ~Instance();

        VkInstance handle() {
            return mInstance;
        }

    private:
        VkInstance mInstance = VK_NULL_HANDLE;
    };
}