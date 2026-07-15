#include <compute/pipeline.hpp>
#include <compute/device.hpp>

#include <spdlog/spdlog.h>
#include <volk.h>

#include <fstream>

namespace Compute {
    ShaderModule::ShaderModule(const std::string& filepath) {
        spdlog::debug("opening shader module {}", filepath);

        std::fstream file(filepath, file.binary | file.ate);
        if (!file.is_open()) {
            spdlog::error("failed to open shader module `{}`", filepath);
            throw std::runtime_error("failed to open shader file");
        }

        size_t filesize = file.tellg();
        file.seekg(0);

        std::string code(filesize, '\0');
        file.read(code.data(), filesize);
        
        VkShaderModuleCreateInfo moduleCi = {};
        moduleCi.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        moduleCi.codeSize = filesize;
        moduleCi.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkResult result = vkCreateShaderModule(mDevice->handle(), &moduleCi, nullptr, &mModule);
        if(result != VK_SUCCESS) {
            spdlog::error("failed to create shader module: {}", static_cast<uint32_t>(result));
            throw std::runtime_error("failed to create shader module");
        }

        spdlog::debug("shader module created");
    }

    ShaderModule::~ShaderModule() {
        if(mModule != VK_NULL_HANDLE) {
            vkDestroyShaderModule(mDevice->handle(), mModule, nullptr);
            spdlog::debug("destroyed shader module");
        }
    }

    Pipeline::Pipeline(std::shared_ptr<Device> device) : mDevice(std::move(device)) {
        ShaderModule computeModule(COMPUTE_SPV_PATH);

        VkPipelineLayoutCreateInfo layoutCi = {};
        layoutCi.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutCi.pushConstantRangeCount = 0;
        layoutCi.setLayoutCount = 0;

        VkResult result = vkCreatePipelineLayout(mDevice->handle(), &layoutCi, nullptr, &mLayout);
        if(result != VK_SUCCESS) {
            spdlog::error("failed to create compute pipeline layout: {}", static_cast<uint32_t>(result));
            throw std::runtime_error("failed to create compute pipeline layout");
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

            spdlog::error("failed to create compute pipeline: {}", static_cast<uint32_t>(result));
            throw std::runtime_error("failed to create compute pipeline");
        }
    }

    Pipeline::~Pipeline() {
        if (mPipeline != VK_NULL_HANDLE) {
            vkDestroyPipeline(mDevice->handle(), mPipeline, nullptr);
            mPipeline = VK_NULL_HANDLE;

            spdlog::debug("destroyed pipeline");
        }

        destroyPipelineLayout();
    }

    void Pipeline::destroyPipelineLayout() {
        if (mLayout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(mDevice->handle(), mLayout, nullptr);
            mLayout = VK_NULL_HANDLE;

            spdlog::debug("destroyed pipeline layout");
        }
    }
}