#pragma once

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#include <compute/spirv_reflect.h>

#include <memory>

namespace Compute {
    class Device;

    class Resource {
    public:
        virtual ~Resource() = default;
    };

    template<typename T, typename... Args>
    concept ConstructableResource = std::derived_from<Resource, T> && requires(T resource, Args&&... args) {
        T(std::forward<Args>(args)...);
    };

    class ReflectLayout {
    public:
        ReflectLayout() = default;
        ReflectLayout(SpvReflectShaderModule module);
        ~ReflectLayout();

        template<typename R, typename... Args> 
        requires ConstructableResource<R, std::shared_ptr<Device>, Args...>
        void bind(Args&&... args) {

        }

    private:
    
    };
}