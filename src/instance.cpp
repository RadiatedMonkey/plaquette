#include <compute/instance.hpp>

#include <volk.h>
#include <spdlog/spdlog.h>

namespace Compute {
    Instance::Instance() {
        VkResult result = volkInitialize();
        if (result != VK_SUCCESS) {
            throw std::runtime_error("volkInitialize failed");
        }

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Compute";
        appInfo.applicationVersion = 1;

        VkInstanceCreateInfo instanceCi = {};
        instanceCi.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCi.pNext = nullptr;
        instanceCi.enabledExtensionCount = std::size(ENABLED_INSTANCE_EXTENSIONS);
        instanceCi.ppEnabledExtensionNames = ENABLED_INSTANCE_EXTENSIONS;
        instanceCi.enabledLayerCount = std::size(ENABLED_INSTANCE_LAYERS);
        instanceCi.ppEnabledLayerNames = ENABLED_INSTANCE_LAYERS;
        instanceCi.pApplicationInfo = &appInfo;

        result = vkCreateInstance(&instanceCi, nullptr, &mInstance);
        if (result != VK_SUCCESS) {
            spdlog::error("failed to create instance: {}", static_cast<uint32_t>(result));
            throw std::runtime_error("failed to create instance");
        }

        volkLoadInstance(mInstance);

        spdlog::debug("instance created");
    }

    Instance::~Instance() {
        if (mInstance != VK_NULL_HANDLE) {
            vkDestroyInstance(mInstance, nullptr);
            mInstance = VK_NULL_HANDLE;

            volkFinalize();

            spdlog::debug("instance destroyed");
        }
    }
}