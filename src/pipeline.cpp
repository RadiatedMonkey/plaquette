#include <compute/pipeline.hpp>
#include <compute/device.hpp>
#include <compute/spirv_reflect.h>

#include <spdlog/spdlog.h>
#include <volk.h>

#include <fstream>

namespace Compute {
    ReflectableShader::ReflectableShader(std::shared_ptr<Device> device, const std::string& filepath) : mDevice(std::move(device)) {
        spdlog::debug("Opening shader module {}", filepath);

        std::ifstream file(filepath, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            spdlog::error("Failed to open shader module file `{}`", filepath);
            throw std::runtime_error("Failed to open shader file");
        }

        size_t filesize = file.tellg();
        file.seekg(0);

        std::vector<uint32_t> code(filesize / sizeof(uint32_t));
        file.read(reinterpret_cast<char*>(code.data()), filesize);
        
        SpvReflectResult spResult = spvReflectCreateShaderModule(filesize, code.data(), &mReflectModule);
        if (spResult != SPV_REFLECT_RESULT_SUCCESS) {
            spdlog::error("Failed to load shader module using SPIRV-Reflect: {}", static_cast<uint32_t>(spResult));
            throw std::runtime_error("Failed to load shader module using SPIRV-Reflect");
        }

        VkShaderModuleCreateInfo moduleCi = {};
        moduleCi.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        moduleCi.codeSize = filesize;
        moduleCi.pCode = code.data();

        VkResult result = vkCreateShaderModule(mDevice->handle(), &moduleCi, nullptr, &mModule);
        if (result != VK_SUCCESS) {
            destroyReflectModule();

            spdlog::error("Failed to create shader module: {}", static_cast<uint32_t>(result));
            throw std::runtime_error("Failed to create shader module");
        }
        
        spdlog::debug("Created shader module");
    }

    ReflectableShader::~ReflectableShader() {
        if(mModule != VK_NULL_HANDLE) {
            vkDestroyShaderModule(mDevice->handle(), mModule, nullptr);
            spdlog::debug("Destroyed shader module");
        }

        destroyReflectModule();
    }

    void ReflectableShader::destroyReflectModule() {
        spvReflectDestroyShaderModule(&mReflectModule);
        spdlog::debug("Destroyed reflection shader module");
    }

    Pipeline::Pipeline(std::shared_ptr<Device> device) : mDevice(device) {
        
    }

    Pipeline::~Pipeline() {
        if (mPipeline != VK_NULL_HANDLE) {
            vkDestroyPipeline(mDevice->handle(), mPipeline, nullptr);
            mPipeline = VK_NULL_HANDLE;

            spdlog::debug("Destroyed pipeline");
        }

        destroyPipelineLayout();
        destroySetLayout();
    }

    void Pipeline::destroyPipelineLayout() {
        if (mLayout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(mDevice->handle(), mLayout, nullptr);
            mLayout = VK_NULL_HANDLE;

            spdlog::debug("Destroyed pipeline layout");
        }
    }

    void Pipeline::destroySetLayout() {
        if (mSetLayout != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(mDevice->handle(), mSetLayout, nullptr);
            mSetLayout = VK_NULL_HANDLE;

            spdlog::debug("Destroyed descriptor set layout");
        }
    }
}