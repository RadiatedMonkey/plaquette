#include <plaquette/vulkan/pipeline.hpp>
#include <plaquette/vulkan/device.hpp>
#include <plaquette/vulkan/spirv_reflect.h>
#include <plaquette/util.hpp>

#include <spdlog/spdlog.h>
#include <volk.h>

#include <array>
#include <fstream>

static constexpr uint32_t MAX_BINDLESS_RESOURCES = 100;

namespace Plaq {
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

        CHECK_VKRESULT(
            vkCreateShaderModule(mDevice->handle(), &moduleCi, nullptr, &mModule),
            "Failed to create shader module",
            {
                destroyReflectModule();
            }
        );

        spdlog::debug("Created shader module");
    }

    ReflectableShader::ReflectableShader(ReflectableShader&& other) noexcept :
        mDevice(std::move(other.mDevice)), mReflectModule(other.mReflectModule), mModule(other.mModule)
    {
        other.mModule = VK_NULL_HANDLE;
    }

    ReflectableShader::~ReflectableShader() {
        if(mModule != VK_NULL_HANDLE) {
            vkDestroyShaderModule(mDevice->handle(), mModule, nullptr);
            spdlog::debug("Destroyed shader module");

            // In here since we cannot easily set `mReflectModule` to some kind of null pointer.
            destroyReflectModule();
        }
    }

    void ReflectableShader::destroyReflectModule() {
        spvReflectDestroyShaderModule(&mReflectModule);
        spdlog::debug("Destroyed reflection shader module");
    }

    Pipeline::Pipeline(std::shared_ptr<Device> device, const PipelineConfig& config) : mDevice(std::move(device)) {
        static constexpr uint32_t DESCRIPTOR_TYPE_COUNT = 2;

        std::array<VkDescriptorSetLayoutBinding, DESCRIPTOR_TYPE_COUNT> bindings = {};
        std::array<VkDescriptorBindingFlags, DESCRIPTOR_TYPE_COUNT> flags = {};
        std::array<VkDescriptorType, DESCRIPTOR_TYPE_COUNT> descriptorTypes = {
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
        };

        for (uint32_t i = 0; i < bindings.size(); i++) {
            bindings[i].binding = i;
            bindings[i].descriptorType = descriptorTypes[i];
            bindings[i].descriptorCount = MAX_BINDLESS_RESOURCES;
            bindings[i].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

            flags[i] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
        }

        VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsCi = {};
        bindingFlagsCi.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
        bindingFlagsCi.bindingCount = DESCRIPTOR_TYPE_COUNT;
        bindingFlagsCi.pBindingFlags = flags.data();

        VkDescriptorSetLayoutCreateInfo setLayoutCi = {};
        setLayoutCi.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        setLayoutCi.bindingCount = bindings.size();
        setLayoutCi.pBindings = bindings.data();
        setLayoutCi.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
        setLayoutCi.pNext = &bindingFlagsCi;

        CHECK_VKRESULT(
            vkCreateDescriptorSetLayout(mDevice->handle(), &setLayoutCi, nullptr, &mBindlessLayout),
            "Failed to create bindless descriptor set layout",
            {
                destroyResources();
            }
        );

        spdlog::debug("Created descriptor set layout");

        std::array<VkDescriptorPoolSize, DESCRIPTOR_TYPE_COUNT> poolSizes = {};
        for (uint32_t i = 0; i < poolSizes.size(); i++) {
            poolSizes[i].type = descriptorTypes[i];
            poolSizes[i].descriptorCount = MAX_BINDLESS_RESOURCES;
        }

        VkDescriptorPoolCreateInfo poolCi = {};
        poolCi.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolCi.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
        poolCi.poolSizeCount = poolSizes.size();
        poolCi.pPoolSizes = poolSizes.data();
        poolCi.maxSets = 1;

        CHECK_VKRESULT(
            vkCreateDescriptorPool(mDevice->handle(), &poolCi, nullptr, &mDescriptorPool),
            "Failed to create descriptor pool",
            {
                destroyResources();
            }
        );

        spdlog::debug("Created descriptor pool");

        VkDescriptorSetAllocateInfo allocCi = {};
        allocCi.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocCi.descriptorPool = mDescriptorPool;
        allocCi.descriptorSetCount = 1;
        allocCi.pSetLayouts = &mBindlessLayout;

        CHECK_VKRESULT(
            vkAllocateDescriptorSets(mDevice->handle(), &allocCi, &mBindlessSet),
            "Failed to allocate descriptor set",
            {
                destroyResources();
            }
        );

        spdlog::debug("Allocated bindless descriptor set");

        ReflectableShader shader(mDevice, config.shaderPath);

        SpvReflectShaderModule module = shader.reflectHandle();
        mReflectLayout = ReflectLayout(module);

        std::vector<VkPushConstantRange> pushConstants(module.push_constant_block_count);
        for (uint32_t i = 0; i < module.push_constant_block_count; i++) {
            VkPushConstantRange pushConstant = {};
            pushConstant.offset = module.push_constant_blocks[i].offset;
            pushConstant.size = module.push_constant_blocks[i].size;
            pushConstant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

            pushConstants[i] = pushConstant;
        }

        spdlog::info("Registered {} push constant block(s)", pushConstants.size());

        VkPipelineLayoutCreateInfo layoutCi = {};
        layoutCi.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutCi.pushConstantRangeCount = pushConstants.size();
        layoutCi.pPushConstantRanges = pushConstants.data();
        layoutCi.setLayoutCount = 1;
        layoutCi.pSetLayouts = &mBindlessLayout;

        CHECK_VKRESULT(
            vkCreatePipelineLayout(mDevice->handle(), &layoutCi, nullptr, &mLayout),
            "Failed to create compute pipeline layout",
            {
                destroyResources();
            }
        );

        spdlog::debug("Created compute pipeline layout");

        VkPipelineShaderStageCreateInfo stageCi = {};
        stageCi.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stageCi.module = shader.handle();
        stageCi.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        stageCi.pName = shader.reflectHandle().entry_point_name;

        VkComputePipelineCreateInfo pipelineCi = {};
        pipelineCi.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        pipelineCi.layout = mLayout;
        pipelineCi.stage = stageCi;

        CHECK_VKRESULT(
            vkCreateComputePipelines(mDevice->handle(), VK_NULL_HANDLE, 1, &pipelineCi, nullptr, &mPipeline),
            "Failed to create compute pipeline",
            {
                destroyResources();
            }
        );

        spdlog::debug("Created compute pipeline");
    }

    Pipeline::Pipeline(Pipeline&& other) noexcept :
        mDevice(std::move(other.mDevice)), mPipeline(other.mPipeline), mLayout(other.mLayout),
        mReflectLayout(std::move(other.mReflectLayout)),
        mDescriptorPool(other.mDescriptorPool), mBindlessLayout(other.mBindlessLayout), mBindlessSet(other.mBindlessSet)
     {
        other.mPipeline = VK_NULL_HANDLE;
        other.mLayout = VK_NULL_HANDLE;
        other.mDescriptorPool = VK_NULL_HANDLE;
        other.mBindlessLayout = VK_NULL_HANDLE;
        other.mBindlessSet = VK_NULL_HANDLE;
    }

    Pipeline::~Pipeline() {
        destroyResources();
    }

    VkPipelineLayout Pipeline::layout() {
        return mLayout;
    }

    VkPipeline Pipeline::handle() {
        return mPipeline;
    }

    void Pipeline::destroyResources() {
        if (mPipeline != VK_NULL_HANDLE) {
            vkDestroyPipeline(mDevice->handle(), mPipeline, nullptr);
            mPipeline = VK_NULL_HANDLE;

            spdlog::debug("Destroyed pipeline");
        }

        if (mLayout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(mDevice->handle(), mLayout, nullptr);
            mLayout = VK_NULL_HANDLE;

            spdlog::debug("Destroyed pipeline layout");
        }

        if (mBindlessLayout != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(mDevice->handle(), mBindlessLayout, nullptr);
            mBindlessLayout = VK_NULL_HANDLE;

            spdlog::debug("Destroyed bindless descriptor set layout");
        }

        if (mBindlessLayout != VK_NULL_HANDLE) {
            vkFreeDescriptorSets(mDevice->handle(), mDescriptorPool, 1, &mBindlessSet);
            mBindlessSet = VK_NULL_HANDLE;

            spdlog::debug("Freed bindless descriptor set");
        }

        if (mDescriptorPool != VK_NULL_HANDLE) {
            vkDestroyDescriptorPool(mDevice->handle(), mDescriptorPool, nullptr);
            mDescriptorPool = VK_NULL_HANDLE;

            spdlog::debug("Destroyed descriptor pool");
        }
    }
}
