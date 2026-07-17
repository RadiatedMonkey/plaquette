#pragma once

#include <string>
#include <memory>

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

namespace Compute {
    static constexpr const char* COMPUTE_SPV_PATH = BUILD_DIR "/shaders/compute.spv";

    class Device;

    class ShaderModule {
    public:
        explicit ShaderModule(std::shared_ptr<Device> device, const std::string& filepath);
        ~ShaderModule();

        VkShaderModule handle() {
            return mModule;
        }

    private:
        std::shared_ptr<Device> mDevice = nullptr;
        VkShaderModule mModule = VK_NULL_HANDLE;
    };

    class Pipeline {
    public:
        Pipeline(std::shared_ptr<Device> device);
        ~Pipeline();

    private:
        void destroyPipelineLayout();

        std::shared_ptr<Device> mDevice = nullptr;

        VkPipelineLayout mLayout = VK_NULL_HANDLE;
        VkPipeline mPipeline = VK_NULL_HANDLE;
    };
}