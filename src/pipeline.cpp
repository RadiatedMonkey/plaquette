#include <compute/pipeline.hpp>
#include <compute/device.hpp>

#include <spdlog/spdlog.h>
#include <volk.h>

#include <fstream>

namespace Compute {
    ShaderModule::ShaderModule(std::shared_ptr<Device> device, const std::string& filepath) : mDevice(std::move(device)) {
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
        
        VkShaderModuleCreateInfo moduleCi = {};
        moduleCi.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        moduleCi.codeSize = filesize;
        moduleCi.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkResult result = vkCreateShaderModule(mDevice->handle(), &moduleCi, nullptr, &mModule);
        if(result != VK_SUCCESS) {
            spdlog::error("Failed to create shader module: {}", static_cast<uint32_t>(result));
            throw std::runtime_error("Failed to create shader module");
        }

        spdlog::debug("Created shader module");
    }

    ShaderModule::~ShaderModule() {
        if(mModule != VK_NULL_HANDLE) {
            vkDestroyShaderModule(mDevice->handle(), mModule, nullptr);
            spdlog::debug("Destroyed shader module");
        }
    }

    Pipeline::Pipeline(std::shared_ptr<Device> device) : mDevice(device) {
        ShaderModule computeModule(std::move(device), COMPUTE_SPV_PATH);

        VkPipelineLayoutCreateInfo layoutCi = {};
        layoutCi.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutCi.pushConstantRangeCount = 0;
        layoutCi.setLayoutCount = 0;

        VkResult result = vkCreatePipelineLayout(mDevice->handle(), &layoutCi, nullptr, &mLayout);
        if(result != VK_SUCCESS) {
            spdlog::error("Failed to create compute pipeline layout: {}", static_cast<uint32_t>(result));
            throw std::runtime_error("Failed to create compute pipeline layout");
        }

        VkPipelineShaderStageCreateInfo stageCi = {};
        stageCi.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stageCi.module = computeModule.handle();
        stageCi.pName = "main";
        stageCi.stage = VK_SHADER_STAGE_COMPUTE_BIT;

        VkComputePipelineCreateInfo pipelineCi = {};
        pipelineCi.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        pipelineCi.stage = stageCi;
        pipelineCi.layout = mLayout;

        result = vkCreateComputePipelines(mDevice->handle(), VK_NULL_HANDLE, 1, &pipelineCi, nullptr, &mPipeline);
        if (result != VK_SUCCESS) {
            destroyPipelineLayout();

            spdlog::error("Failed to create compute pipeline: {}", static_cast<uint32_t>(result));
            throw std::runtime_error("Failed to create compute pipeline");
        }
    }

    Pipeline::~Pipeline() {
        if (mPipeline != VK_NULL_HANDLE) {
            vkDestroyPipeline(mDevice->handle(), mPipeline, nullptr);
            mPipeline = VK_NULL_HANDLE;

            spdlog::debug("Destroyed pipeline");
        }

        destroyPipelineLayout();
    }

    void Pipeline::destroyPipelineLayout() {
        if (mLayout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(mDevice->handle(), mLayout, nullptr);
            mLayout = VK_NULL_HANDLE;

            spdlog::debug("Destroyed pipeline layout");
        }
    }
}