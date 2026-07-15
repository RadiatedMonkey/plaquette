#include <compute/pipeline.hpp>
#include <compute/device.hpp>

#include <volk.h>

#include <iostream>
#include <fstream>

namespace Compute {
    class ShaderModule {
    public:
        explicit ShaderModule(const std::string& filepath) {
            std::cout << "Opening file " << filepath << std::endl;

            std::fstream file(filepath, std::ios::binary | std::ios::ate);
            size_t filesize = file.tellg();
            file.seekg(0);

            std::cout << "Compute module file size is " << filesize << std::endl;

            std::string code(filesize, '\0');
            file.read(code.data(), filesize);
            
            VkShaderModuleCreateInfo moduleCi = {};
            moduleCi.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            moduleCi.codeSize = filesize;
            moduleCi.pCode = reinterpret_cast<const uint32_t*>(code.data());

            VkResult result = vkCreateShaderModule(mDevice->handle(), &moduleCi, nullptr, &mModule);
            if(result != VK_SUCCESS) {
                throw std::runtime_error("failed to create shader module");
            }
        }

        ~ShaderModule() {
            if(mModule != VK_NULL_HANDLE) {
                vkDestroyShaderModule(mDevice->handle(), mModule, nullptr);
                std::cout << "Destroyed shader module" << std::endl;
            }
        }

        VkShaderModule handle() {
            return mModule;
        }

    private:
        std::shared_ptr<Device> mDevice;
        VkShaderModule mModule = VK_NULL_HANDLE;
    };

    class ComputePipeline {
    public:
        explicit ComputePipeline(std::shared_ptr<Device> device) {
            ShaderModule computeModule(COMPUTE_SPV_PATH);

            VkPipelineLayoutCreateInfo layoutCi = {};
            layoutCi.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            layoutCi.pushConstantRangeCount = 0;
            layoutCi.setLayoutCount = 0;

            VkResult result = vkCreatePipelineLayout(mDevice->handle(), &layoutCi, nullptr, &mPipelineLayout);
            if(result != VK_SUCCESS) {
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
            pipelineCi.layout = mPipelineLayout;

            result = vkCreateComputePipelines(mDevice->handle(), VK_NULL_HANDLE, 1, &pipelineCi, nullptr, &mPipeline);
            if (result != VK_SUCCESS) {
                vkDestroyPipelineLayout(mDevice->handle(), mPipelineLayout, nullptr);
                std::cout << "Destroyed pipeline layout" << std::endl;
                throw std::runtime_error("failed to create compute pipeline");
            }
        }

        ~ComputePipeline() {
            if (mPipeline != VK_NULL_HANDLE) {
                vkDestroyPipeline(mDevice->handle(), mPipeline, nullptr);
                mPipeline = VK_NULL_HANDLE;

                std::cout << "Destroyed compute pipeline" << std::endl;
            }

            destroyPipelineLayout();
        }

    private:
        void destroyPipelineLayout() {
            if (mPipelineLayout != VK_NULL_HANDLE) {
                vkDestroyPipelineLayout(mDevice->handle(), mPipelineLayout, nullptr);
                mPipelineLayout = VK_NULL_HANDLE;

                std::cout << "Destroyed pipeline layout" << std::endl;
            }
        }

        std::shared_ptr<Device> mDevice;

        VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;
        VkPipeline mPipeline = VK_NULL_HANDLE;
    };
}