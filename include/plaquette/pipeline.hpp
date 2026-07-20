#pragma once

#include <string>
#include <memory>
#include <type_traits>

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#include <plaquette/reflection.hpp>
#include <plaquette/spirv_reflect.h>

namespace Plaq {
    class Device;

    class ReflectableShader {
    public:
        ReflectableShader(std::shared_ptr<Device> device, const std::string& filepath);
        ReflectableShader(ReflectableShader&& other) noexcept;
        ReflectableShader(const ReflectableShader&) = delete;
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

    struct PipelineConfig {
        std::string shaderPath;
    };

    class Pipeline {
    public:
        Pipeline(Pipeline&& other) noexcept;
        Pipeline(const Pipeline&) = delete;
        ~Pipeline();

        VkPipeline handle();
        VkPipelineLayout layout();

        ReflectLayout& resources() {
            return mReflectLayout;
        }

        const ReflectLayout& resources() const {
            return mReflectLayout;
        }

    private:
        friend Device;

        Pipeline(std::shared_ptr<Device> device, const PipelineConfig& config);

        void destroyResources();

        std::shared_ptr<Device> mDevice = nullptr;

        ReflectLayout mReflectLayout;

        VkDescriptorSetLayout mBindlessLayout = VK_NULL_HANDLE;
        VkDescriptorSet mBindlessSet = VK_NULL_HANDLE;
        VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;
        VkPipelineLayout mLayout = VK_NULL_HANDLE;
        VkPipeline mPipeline = VK_NULL_HANDLE;
    };
}