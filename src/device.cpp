#include <compute/device.hpp>
#include <compute/instance.hpp>

#include <volk.h>
#include <spdlog/spdlog.h>

#include <vector>

namespace Compute {
    static constexpr const char* ENABLED_DEVICE_EXTENSIONS[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
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
            properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;

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

        for (uint32_t family = 0; family < familyCount; family++) {
            auto queue = families[family];

            bool supportsCompute = (queue.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0;
            bool supportsTransfer = (queue.queueFlags & VK_QUEUE_TRANSFER_BIT) != 0;

            if (supportsCompute && supportsTransfer) {
                mQueueFamilyIndex = family;
                break;
            }
        }

        if (mQueueFamilyIndex == UINT32_MAX) {
            spdlog::error("Could not find queue with compute and transfer capabilities");
            throw std::runtime_error("Could not find suitable compute queue");
        }

        float queuePriority = 1.0f;

        VkDeviceQueueCreateInfo queueCi = {};
        queueCi.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCi.queueFamilyIndex = mQueueFamilyIndex;
        queueCi.queueCount = 1;
        queueCi.pQueuePriorities = &queuePriority;

        VkPhysicalDeviceSynchronization2Features sync2Features = {};
        sync2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;

        VkPhysicalDeviceFeatures2 enabledFeatures = {};
        enabledFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        enabledFeatures.pNext = &sync2Features;

        vkGetPhysicalDeviceFeatures2(chosenAdapter, &enabledFeatures);

        if (!sync2Features.synchronization2) {
            throw std::runtime_error("This device does not support synchronization2");
        }

        sync2Features.synchronization2 = VK_TRUE;

        VkDeviceCreateInfo deviceCi = {};
        deviceCi.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCi.queueCreateInfoCount = 1;
        deviceCi.pQueueCreateInfos = &queueCi;
        deviceCi.enabledExtensionCount = std::size(ENABLED_DEVICE_EXTENSIONS);
        deviceCi.ppEnabledExtensionNames = ENABLED_DEVICE_EXTENSIONS;
        deviceCi.enabledLayerCount = 0; // Device layers are deprecated
        deviceCi.pNext = &enabledFeatures;

        result = vkCreateDevice(chosenAdapter, &deviceCi, nullptr, &mDevice);
        if (result != VK_SUCCESS) {
            spdlog::error("Failed to create logical device: {}", static_cast<uint32_t>(result));
            throw std::runtime_error("Failed to create logical device");
        }

        spdlog::debug("Created device");

        mFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        mMemProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;

        vkGetPhysicalDeviceFeatures2(chosenAdapter, &mFeatures);
        vkGetPhysicalDeviceMemoryProperties2(chosenAdapter, &mMemProperties);

        VkDeviceQueueInfo2 queueInfo = {};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2;
        queueInfo.queueFamilyIndex = mQueueFamilyIndex;
        queueInfo.queueIndex = 0;

        vkGetDeviceQueue2(mDevice, &queueInfo, &mQueue);

        spdlog::debug("Created logical device");

        VkCommandPoolCreateInfo poolCi = {};
        poolCi.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolCi.queueFamilyIndex = mQueueFamilyIndex;

        result = vkCreateCommandPool(mDevice, &poolCi, nullptr, &mPool);
        if (result != VK_SUCCESS) {
            destroyDevice();

            spdlog::error("Failed to create command pool: {}", static_cast<uint32_t>(result));
            throw std::runtime_error("Failed to create command pool");
        }
    }

    Device::~Device() {
        if (mPool != VK_NULL_HANDLE) {
            vkDestroyCommandPool(mDevice, mPool, nullptr);
            spdlog::debug("Destroyed command pool");
        }

        destroyDevice();
    }

    void Device::destroyDevice() {
        if (mDevice != VK_NULL_HANDLE) {
            vkDestroyDevice(mDevice, nullptr);
            spdlog::debug("Destroyed logical device");
        }
    }

    VkDevice Device::handle() {
        return mDevice;
    }

    Commands Device::record() {
        VkCommandBufferAllocateInfo bufferCi = {};
        bufferCi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        bufferCi.commandPool = mPool;
        bufferCi.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        bufferCi.commandBufferCount = 1;

        std::shared_ptr<Device> device = shared_from_this();

        VkCommandBuffer buffer = VK_NULL_HANDLE;
        VkResult result = vkAllocateCommandBuffers(mDevice, &bufferCi, &buffer);
        if (result != VK_SUCCESS) {
            spdlog::error("Failed to allocate command buffers: {}", static_cast<uint32_t>(result));
            throw std::runtime_error("Failed to allocate command buffers");
        }

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        result = vkBeginCommandBuffer(buffer, &beginInfo);
        if (result != VK_SUCCESS) {
            vkFreeCommandBuffers(mDevice, mPool, 1, &buffer);
            spdlog::debug("Freed command buffer");

            spdlog::error("Failed to begin command buffer: {}", static_cast<uint32_t>(result));
            throw std::runtime_error("Failed to begin command buffer");
        }

        return Commands(std::move(device), buffer);
    }

    VkFence Device::submit(Commands commands) {
        VkResult result = vkEndCommandBuffer(commands.handle());
        if (result != VK_SUCCESS) {
            spdlog::error("Failed to end command buffer: {}", static_cast<uint32_t>(result));
            throw std::runtime_error("Failed to end command buffer");
        }

        VkFenceCreateInfo fenceCi = {};
        fenceCi.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

        VkFence fence = VK_NULL_HANDLE;
        result = vkCreateFence(mDevice, &fenceCi, nullptr, &fence);
        if (result != VK_SUCCESS) {
            spdlog::error("Failed to create fence: {}", static_cast<uint32_t>(result));
            throw std::runtime_error("Failed to create fence");
        }

        VkCommandBufferSubmitInfo cmdBufferInfo = {};
        cmdBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        cmdBufferInfo.commandBuffer = commands.handle();

        VkSubmitInfo2 submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submitInfo.commandBufferInfoCount = 1;
        submitInfo.pCommandBufferInfos = &cmdBufferInfo;

        result = vkQueueSubmit2(mQueue, 1, &submitInfo, fence);
        if (result != VK_SUCCESS) {
            spdlog::error("Failed to submit command buffer: {}", static_cast<uint32_t>(result));
            throw std::runtime_error("Failed to submit command buffer");
        }

        return fence;
    }

    VkCommandPool Device::pool() {
        return mPool;
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