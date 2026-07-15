#pragma once

#include <memory>

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

namespace std {
    class string;
}

namespace Compute {
    static constexpr const char* COMPUTE_SPV_PATH = "compute.spv";

    class Device;

    class ShaderModule {
    public:
        explicit ShaderModule(const std::string& filepath);
        ~ShaderModule();

        VkShaderModule handle() {
            return mModule;
        }

    private:
        std::shared_ptr<Device> mDevice = nullptr;
        VkShaderModule mModule = VK_NULL_HANDLE;
    };
}