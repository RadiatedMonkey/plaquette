#pragma once

#include <memory>

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

namespace Compute {
    static constexpr const char* ENABLED_INSTANCE_EXTENSIONS[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME
    };

    static constexpr const char* ENABLED_INSTANCE_LAYERS[] = {
        "VK_LAYER_KHRONOS_validation"
    };

    class Device;

    class Instance : public std::enable_shared_from_this<Instance> {
    public:
        static std::shared_ptr<Instance> create();

        ~Instance();

        std::shared_ptr<Device> createDevice();

        VkInstance handle() {
            return mInstance;
        }

    private:
        Instance();

        VkInstance mInstance = VK_NULL_HANDLE;
    };
}