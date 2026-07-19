#include <compute/instance.hpp>
#include <compute/device.hpp>

#include <volk.h>
#include <spdlog/spdlog.h>

namespace Compute {
    std::shared_ptr<Instance> Instance::create() {
        // Due to private constructor, make_shared does not work.
        return std::shared_ptr<Instance>(new Instance());
    }

    Instance::Instance() {
        VkResult result = volkInitialize();
        if (result != VK_SUCCESS) {
            throw std::runtime_error("volkInitialize failed");
        }

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Compute";
        appInfo.applicationVersion = 1;
        appInfo.apiVersion = VK_API_VERSION_1_4;

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
            spdlog::error("Failed to create instance: {}", static_cast<uint32_t>(result));
            throw std::runtime_error("Failed to create instance");
        }

        volkLoadInstance(mInstance);

        spdlog::debug("Created instance");
    }

    Instance::~Instance() {
        if (mInstance != VK_NULL_HANDLE) {
            vkDestroyInstance(mInstance, nullptr);
            mInstance = VK_NULL_HANDLE;

            volkFinalize();

            spdlog::debug("Destroyed instance");
        }
    }

    std::shared_ptr<Device> Instance::createDevice() {
        // make_shared cannot access private Device constructor.
        return std::shared_ptr<Device>(new Device(shared_from_this()));
    }
}