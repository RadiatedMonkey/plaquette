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
            spdlog::error(
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

        mSlangModule->findEntryPointByName(config.entryPoint, mEntryPoint.writeRef());

        std::array<slang::IComponentType*, 2> componentTypes = {
            mSlangModule, mEntryPoint
        };

        diagnosticBlob = nullptr;
        SlangResult result = session->createCompositeComponentType(
            componentTypes.data(), componentTypes.size(),
            mComposedProgram.writeRef(), diagnosticBlob.writeRef()
        );

        if (diagnosticBlob != nullptr) {
            spdlog::error(
                "Info while creating composite component type for `{}`: {}",
                config.moduleName,
                reinterpret_cast<const char*>(diagnosticBlob->getBufferPointer())
            );
        }

        LOG_SRESULT(result, "Failed to create composite component type");

        diagnosticBlob = nullptr;
        result = mComposedProgram->getEntryPointCode(
            0, 0, mSpirvCode.writeRef(), diagnosticBlob.writeRef()
        );

        if (diagnosticBlob != nullptr) {
            spdlog::error(
                "Info while loading entry point SPIR-V `{}`: {}",
                config.moduleName,
                reinterpret_cast<const char*>(diagnosticBlob->getBufferPointer())
            );
        }

        LOG_SRESULT(result, "Failed to load SPIR-V entrypoint");

        spdlog::trace("Loaded SPIR-V entrypoint");
    }

    Shader::Shader(Shader&& other) noexcept :
        mDevice(std::move(other.mDevice)),
        mSlangModule(std::move(other.mSlangModule)),
        mSpirvCode(std::move(other.mSpirvCode)),
        mComposedProgram(std::move(other.mComposedProgram)),
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
}
