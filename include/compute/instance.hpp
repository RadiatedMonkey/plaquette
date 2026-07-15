#pragma once

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

namespace Compute {
    static constexpr const char* ENABLED_INSTANCE_EXTENSIONS[] = {
    "VK_KHR_surface",
    "VK_KHR_win32_surface"
};

    static constexpr const char* ENABLED_INSTANCE_LAYERS[] = {
        "VK_LAYER_KHRONOS_validation"
    };

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