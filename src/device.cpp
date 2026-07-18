#include <compute/device.hpp>
#include <compute/instance.hpp>

#include <volk.h>
#include <spdlog/spdlog.h>

#include <vector>

namespace Compute {
    static constexpr const char* ENABLED_DEVICE_EXTENSIONS[] = {
        "VK_KHR_swapchain"
    };

    Device::Device(std::shared_ptr<Instance> instance) : mInstance(std::move(instance)) {
        uint32_t count = 0;
        VkResult result = vkEnumeratePhysicalDevices(mInstance->handle(), &count, nullptr);
        if (result != VK_SUCCESS) {
            spdlog::error("Failed to enumerate physical devices: {}", static_cast<uint32_t>(result));
            throw std::runtime_error("Failed to enumerate physical devices");
        }

        if (count == 0) {
            spdlog::error("Found zero physical devices");
            throw std::runtime_error("No adapters found");
        }

        std::vector<VkPhysicalDevice> rawAdapters(count);
        result = vkEnumeratePhysicalDevices(mInstance->handle(), &count, rawAdapters.data());
        if (result != VK_SUCCESS) {
            spdlog::error("Failed to enumerate physical devices: {}", static_cast<uint32_t>(result));
            throw std::runtime_error("Failed to enumerate physical devices");
        }

        VkPhysicalDevice chosenAdapter = rawAdapters[0];
        for (VkPhysicalDevice adapter : rawAdapters) {
            VkPhysicalDeviceProperties2 properties = {};
            vkGetPhysicalDeviceProperties2(adapter, &properties);

            if (properties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                chosenAdapter = adapter;
                mProperties = properties;

                break;
            }
        }

        uint32_t familyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(chosenAdapter, &familyCount, nullptr);

        std::vector<VkQueueFamilyProperties> families(familyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(chosenAdapter, &familyCount, families.data());

        uint32_t chosenQueue = UINT32_MAX;
        for (uint32_t family = 0; family < familyCount; family++) {
            auto queue = families[family];

            bool supportsCompute = (queue.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0;
            bool supportsTransfer = (queue.queueFlags & VK_QUEUE_TRANSFER_BIT) != 0;

            if (supportsCompute && supportsTransfer) {
                chosenQueue = family;
                break;
            }
        }

        if (chosenQueue == UINT32_MAX) {
            spdlog::error("Could not find queue with compute and transfer capabilities");
            throw std::runtime_error("Could not find suitable compute queue");
        }

        float queuePriority = 1.0f;

        VkDeviceQueueCreateInfo queueCi = {};
        queueCi.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCi.queueFamilyIndex = chosenQueue;
        queueCi.queueCount = 1;
        queueCi.pQueuePriorities = &queuePriority;

        VkDeviceCreateInfo deviceCi = {};
        deviceCi.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCi.queueCreateInfoCount = 1;
        deviceCi.pQueueCreateInfos = &queueCi;
        deviceCi.enabledExtensionCount = std::size(ENABLED_DEVICE_EXTENSIONS);
        deviceCi.ppEnabledExtensionNames = ENABLED_DEVICE_EXTENSIONS;
        deviceCi.enabledLayerCount = 0; // Device layers are deprecated

        result = vkCreateDevice(chosenAdapter, &deviceCi, nullptr, &mDevice);
        if (result != VK_SUCCESS) {
            spdlog::error("Failed to create logical device: {}", static_cast<uint32_t>(result));
            throw std::runtime_error("Failed to create logical device");
        }

        vkGetPhysicalDeviceFeatures2(chosenAdapter, &mFeatures);
        vkGetPhysicalDeviceMemoryProperties2(chosenAdapter, &mMemProperties);

        VkDeviceQueueInfo2 queueInfo = {};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2;
        queueInfo.queueFamilyIndex = chosenQueue;
        queueInfo.queueIndex = 0;

        VkQueue rawQueue = VK_NULL_HANDLE;
        vkGetDeviceQueue2(mDevice, &queueInfo, &rawQueue);

        mQueue = Queue(shared_from_this(), rawQueue, chosenQueue);

        spdlog::debug("Created logical device");
    }

    Device::~Device() {
        if (mDevice != VK_NULL_HANDLE) {
            vkDestroyDevice(mDevice, nullptr);
            mDevice = VK_NULL_HANDLE;

            spdlog::debug("Destroyed logical device");
        }
    }

    VkDevice Device::handle() {
        return mDevice;
    }

    Queue& Device::queue() {
        return mQueue;
    }

    const Queue& Device::queue() const {
        return mQueue;
    }

    VkPhysicalDeviceProperties2 Device::properties() {
        return mProperties;
    }

    VkPhysicalDeviceFeatures2 Device::features() {
        return mFeatures;
    }

    VkPhysicalDeviceMemoryProperties2 Device::memProperties() {
        return mMemProperties;
    }
}   