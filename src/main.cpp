#include <cassert>
#include <memory>
#include <vector>
#include <iostream>
#include <fstream>

#include <spdlog/spdlog.h>
#include <GLFW/glfw3.h>
#include <volk.h>

static constexpr int WINDOW_WIDTH = 800;
static constexpr int WINDOW_HEIGHT = 600;
static constexpr const char* WINDOW_TITLE = "Relativistic Raytracer";
static constexpr const char* COMPUTE_SPV_PATH = "compute.spv";

static constexpr const char* ENABLED_INSTANCE_EXTENSIONS[] = {
    "VK_KHR_surface",
    "VK_KHR_win32_surface"
};

static constexpr const char* ENABLED_INSTANCE_LAYERS[] = {
    "VK_LAYER_KHRONOS_validation"
};

static constexpr const char* ENABLED_DEVICE_EXTENSIONS[] = {
    "VK_KHR_swapchain"
};

class Instance {
public:
    Instance() {
        VkResult result = volkInitialize();
        if (result != VK_SUCCESS) {
            throw std::runtime_error("volkInitialize failed");
        }

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Relativistic Raytracer";
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
            throw std::runtime_error("failed to create instance");
        }

        volkLoadInstance(mInstance);

        std::cout << "Instance created" << std::endl;
    }

    ~Instance() {
        if (mInstance != VK_NULL_HANDLE) {
            vkDestroyInstance(mInstance, nullptr);
            mInstance = VK_NULL_HANDLE;

            volkFinalize();

            std::cout << "Instance destroyed" << std::endl;
        }
    }

    VkInstance handle() {
        return mInstance;
    }

private:
    VkInstance mInstance = VK_NULL_HANDLE;
};

class Device {
public:
    Device(std::shared_ptr<Instance> instance) : mInstance(std::move(instance)) {
        uint32_t count = 0;
        VkResult result = vkEnumeratePhysicalDevices(mInstance->handle(), &count, nullptr);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to enumerate physical devices");
        }

        if (count == 0) {
            throw std::runtime_error("no adapters found");
        }

        std::vector<VkPhysicalDevice> rawAdapters(count);
        result = vkEnumeratePhysicalDevices(mInstance->handle(), &count, rawAdapters.data());
        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to enumerate physical devices");
        }

        VkPhysicalDevice chosenAdapter = rawAdapters[0];
        for (VkPhysicalDevice adapter : rawAdapters) {
            VkPhysicalDeviceProperties properties = {};
            vkGetPhysicalDeviceProperties(adapter, &properties);

            if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                chosenAdapter = adapter;
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
            throw std::runtime_error("could not find suitable compute queue");
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
            throw std::runtime_error("failed to create logical device");
        }

        std::cout << "Device created" << std::endl;
    }

    ~Device() {
        if (mDevice != VK_NULL_HANDLE) {
            vkDestroyDevice(mDevice, nullptr);
            mDevice = VK_NULL_HANDLE;

            std::cout << "Device destroyed" << std::endl;
        }
    }
    
    VkDevice handle()
    {
        return mDevice;
    } 

private:
    std::shared_ptr<Instance> mInstance;
    VkDevice mDevice = VK_NULL_HANDLE;
};

int main() {
    auto logger = spdlog::

    try {
        auto instance = std::make_shared<Instance>();
        auto device = std::make_shared<Device>(instance);
        auto pipeline = std::make_shared<ComputePipeline>(device);
    } catch(const std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
        return 1;
    } catch(...) {
        std::cout << "Unknown exception occurred" << std::endl;
        return 1;
    }

    return 0;
}