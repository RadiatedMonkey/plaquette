#pragma once

#include <string>
#include <memory>
#include <type_traits>

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#include <compute/reflection.hpp>
#include <compute/spirv_reflect.h>

namespace Compute {
    static constexpr const char* COMPUTE_SPV_PATH = BUILD_DIR "/shaders/compute.spv";

    class Device;

    class ReflectableShader {
    public:
        ReflectableShader(std::shared_ptr<Device> device, const std::string& filepath);
        ~ReflectableShader();

        VkShaderModule handle() {
            return mModule;
        }

        SpvReflectShaderModule reflectHandle() {
            return mReflectModule;
        }

    private:
        void destroyReflectModule();

        std::shared_ptr<Device> mDevice = nullptr;
        SpvReflectShaderModule mReflectModule;
        VkShaderModule mModule = VK_NULL_HANDLE;
    };

    class Pipeline {
    public:
        Pipeline(std::shared_ptr<Device> device);
        ~Pipeline();

        ReflectLayout& resources() {
            return mReflectLayout;
        }

        const ReflectLayout& resources() const {
            return mReflectLayout;
        }

    private:
        void destroyPipelineLayout();
        void destroySetLayout();

        std::shared_ptr<Device> mDevice = nullptr;

        ReflectLayout mReflectLayout;

        std::vector<VkDescriptorSetLayout> mSetLayouts;
        VkPipelineLayout mLayout = VK_NULL_HANDLE;
        VkPipeline mPipeline = VK_NULL_HANDLE;
    };
}