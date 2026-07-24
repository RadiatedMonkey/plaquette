#pragma once

#include <memory>

#include <slang/slang-com-ptr.h>
#include <slang/slang.h>

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

namespace slang {
    class IModule;
}

namespace Plaq {
    class Device;

    struct ShaderConfig {
        const char* moduleName;
        const char* entryPoint = "main";
    };

    class Shader {
    public:
        Shader(std::shared_ptr<Device> device, const ShaderConfig& config);
        Shader(Shader&& other) noexcept;
        Shader(const Shader&) = delete;
        ~Shader();

    private:
        std::shared_ptr<Device> mDevice;

        Slang::ComPtr<slang::IBlob> mSpirvCode = nullptr;
        Slang::ComPtr<slang::IComponentType> mComposedProgram = nullptr;
        Slang::ComPtr<slang::IEntryPoint> mEntryPoint = nullptr;
        slang::IModule* mSlangModule = nullptr;
        VkShaderModule mModule = VK_NULL_HANDLE;
    };
}
