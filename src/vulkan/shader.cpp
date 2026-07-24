#include <plaquette/vulkan/shader.hpp>
#include <plaquette/vulkan/device.hpp>
#include <plaquette/util.hpp>

#include <array>

#include <volk.h>
#include <spdlog/spdlog.h>

namespace Plaq {
    Shader::Shader(std::shared_ptr<Device> device, const ShaderConfig& config)
        : mDevice(std::move(device))
    {
        Slang::ComPtr<slang::ISession>& session = mDevice->localSlangSession();

        Slang::ComPtr<slang::IBlob> diagnosticBlob;
        mSlangModule = session->loadModule(config.moduleName, diagnosticBlob.writeRef());

        if (diagnosticBlob != nullptr) {
            spdlog::warn(
                "Info while loading module `{}`: {}",
                config.moduleName,
                reinterpret_cast<const char*>(diagnosticBlob->getBufferPointer())
            );
        }

        if (!mSlangModule) {
            spdlog::error("Failed to load shader module `{}`", config.moduleName);
            throw std::runtime_error("Failed to compile shader module");
        }

        spdlog::trace("Loaded module `{}`", config.moduleName);

        SlangResult result;
        if (config.entryPoint == nullptr) {
            // Load first found entrypoint
            result = mSlangModule->getDefinedEntryPoint(0, mEntryPoint.writeRef());
        } else {
            // Load specific entrypoint
            result = mSlangModule->findEntryPointByName(config.entryPoint, mEntryPoint.writeRef());
        }

        LOG_SRESULT(result, "Failed to find entrypoint");

        std::array<slang::IComponentType*, 2> componentTypes = {
            mSlangModule, mEntryPoint
        };

        diagnosticBlob = nullptr;
        Slang::ComPtr<slang::IComponentType> composedProgram = nullptr;

        result = session->createCompositeComponentType(
            componentTypes.data(), componentTypes.size(),
            composedProgram.writeRef(), diagnosticBlob.writeRef()
        );

        if (diagnosticBlob != nullptr) {
            spdlog::warn(
                "Info while creating composite component type for `{}`: {}",
                config.moduleName,
                reinterpret_cast<const char*>(diagnosticBlob->getBufferPointer())
            );
        }

        LOG_SRESULT(result, "Failed to create composite component type");

        diagnosticBlob = nullptr;
        Slang::ComPtr<slang::IBlob> spirvCode = nullptr;

        result = composedProgram->getEntryPointCode(
            0, 0, spirvCode.writeRef(), diagnosticBlob.writeRef()
        );

        if (diagnosticBlob != nullptr) {
            spdlog::warn(
                "Info while loading entry point SPIR-V `{}`: {}",
                config.moduleName,
                reinterpret_cast<const char*>(diagnosticBlob->getBufferPointer())
            );
        }

        LOG_SRESULT(result, "Failed to load SPIR-V entrypoint");

        spdlog::trace("Loaded SPIR-V entrypoint");

        VkShaderModuleCreateInfo moduleCi = {};
        moduleCi.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        moduleCi.codeSize = spirvCode->getBufferSize();
        moduleCi.pCode = reinterpret_cast<const uint32_t*>(spirvCode->getBufferPointer());

        LOG_VKRESULT(
            vkCreateShaderModule(mDevice->handle(), &moduleCi, nullptr, &mModule),
            "Failed to create Vulkan shader module"
        );

        mLinkedProgram = composedProgram;
    }

    Shader::Shader(Shader&& other) noexcept :
        mDevice(std::move(other.mDevice)),
        mSlangModule(std::move(other.mSlangModule)),
        mLinkedProgram(std::move(other.mLinkedProgram)),
        mEntryPoint(std::move(other.mEntryPoint))
    {
        mModule = other.mModule;
        other.mModule = VK_NULL_HANDLE;
    }

    Shader::~Shader() {
        if (mModule != VK_NULL_HANDLE) {
            vkDestroyShaderModule(mDevice->handle(), mModule, nullptr);
            mModule = VK_NULL_HANDLE;

            spdlog::trace("Destroyed shader module");
        }
    }

    VkShaderModule Shader::handle() {
        return mModule;
    }

    std::string Shader::entryPoint() const {
        slang::FunctionReflection* reflection = mEntryPoint->getFunctionReflection();

        const char* entryPointName = reflection->getName();
        return std::string(entryPointName);
    }

    std::vector<VkPushConstantRange> Shader::pushConstantDescriptor() const {
        // TODO
        return {};
    }
}
