#include <cassert>
#include <memory>
#include <vector>
#include <iostream>
#include <fstream>

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

class Device {
public:
    Device(VkPhysicalDevice adapter, uint32_t queue_family) {
        float queue_priority = 1.0f;

        VkDeviceQueueCreateInfo queue_ci = {};
        queue_ci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_ci.queueFamilyIndex = queue_family;
        queue_ci.queueCount = 1;
        queue_ci.pQueuePriorities = &queue_priority;

        VkDeviceCreateInfo device_ci = {};
        device_ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_ci.queueCreateInfoCount = 1;
        device_ci.pQueueCreateInfos = &queue_ci;
        device_ci.enabledExtensionCount = std::size(ENABLED_DEVICE_EXTENSIONS);
        device_ci.ppEnabledExtensionNames = ENABLED_DEVICE_EXTENSIONS;
        device_ci.enabledLayerCount = 0; // Device layers are deprecated

        VkResult result = vkCreateDevice(adapter, &device_ci, nullptr, &m_device);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device");
        }

        std::cout << "Device created" << std::endl;
    }

    ~Device() {
        if (m_device != VK_NULL_HANDLE) {
            vkDestroyDevice(m_device, nullptr);
            m_device = VK_NULL_HANDLE;

            std::cout << "Device destroyed" << std::endl;
        }
    }
    
    VkDevice handle()
    {
        return m_device;
    } 

private:
    VkDevice m_device = VK_NULL_HANDLE;
};

class ShaderModule {
public:
    explicit ShaderModule(const std::string& filepath) {
        std::fstream file(filepath, std::ios::binary | std::ios::ate);
        size_t filesize = file.tellg();
        file.seekg(0);

        std::string code(filesize, '\0');
        file.read(code.data(), filesize);
        
        VkShaderModuleCreateInfo module_ci = {};
        module_ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        module_ci.codeSize = filesize;
        module_ci.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkResult result = vkCreateShaderModule(mDevice->handle(), &module_ci, nullptr, &mModule);
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module");
        }
    }

    ~ShaderModule() {
        if(mModule != VK_NULL_HANDLE) {
            vkDestroyShaderModule(mDevice->handle(), mModule, nullptr);
            std::cout << "Destroyed shader module" << std::endl;
        }
    }

    VkShaderModule handle() {
        return mModule;
    }

private:
    std::shared_ptr<Device> mDevice;
    VkShaderModule mModule = VK_NULL_HANDLE;
};

class ComputePipeline {
public:
    explicit ComputePipeline(std::shared_ptr<Device> device) {
        ShaderModule computeModule(COMPUTE_SPV_PATH);

        // TODO
        throw std::runtime_error("todo");
    }

    ~ComputePipeline() {
        if (m_pipeline != VK_NULL_HANDLE) {
            vkDestroyPipeline(mDevice->handle(), m_pipeline, nullptr);
            m_pipeline = VK_NULL_HANDLE;

            std::cout << "Destroyed compute pipeline" << std::endl;
        }

        destroyPipeline();
    }

    void destroyPipeline() {
        if (m_pipeline_layout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(mDevice->handle(), m_pipeline_layout, nullptr);
            m_pipeline_layout = VK_NULL_HANDLE;

            std::cout << "Destroyed pipeline layout" << std::endl;
        }
    }

private:
    std::shared_ptr<Device> mDevice;

    VkPipelineLayout m_pipeline_layout = VK_NULL_HANDLE;
    VkPipeline m_pipeline = VK_NULL_HANDLE;
};

class Instance {
public:
    Instance() {
        VkResult result = volkInitialize();
        if (result != VK_SUCCESS) {
            throw std::runtime_error("volkInitialize failed");
        }

        VkApplicationInfo app_info = {};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = "Relativistic Raytracer";
        app_info.applicationVersion = 1;

        VkInstanceCreateInfo instance_ci = {};
        instance_ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instance_ci.pNext = nullptr;
        instance_ci.enabledExtensionCount = std::size(ENABLED_INSTANCE_EXTENSIONS);
        instance_ci.ppEnabledExtensionNames = ENABLED_INSTANCE_EXTENSIONS;
        instance_ci.enabledLayerCount = std::size(ENABLED_INSTANCE_LAYERS);
        instance_ci.ppEnabledLayerNames = ENABLED_INSTANCE_LAYERS;
        instance_ci.pApplicationInfo = &app_info;

        result = vkCreateInstance(&instance_ci, nullptr, &m_instance);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance");
        }

        volkLoadInstance(m_instance);

        std::cout << "Instance created" << std::endl;
    }

    ~Instance() {
        if (m_instance != VK_NULL_HANDLE) {
            vkDestroyInstance(m_instance, nullptr);
            m_instance = VK_NULL_HANDLE;

            volkFinalize();

            std::cout << "Instance destroyed" << std::endl;
        }
    }

    [[nodiscard]]
    Device requestDevice() const {
        uint32_t count = 0;
        VkResult result = vkEnumeratePhysicalDevices(m_instance, &count, nullptr);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to enumerate physical devices");
        }

        if (count == 0) {
            throw std::runtime_error("no adapters found");
        }

        std::vector<VkPhysicalDevice> raw_adapters(count);
        result = vkEnumeratePhysicalDevices(m_instance, &count, raw_adapters.data());
        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to enumerate physical devices");
        }

        VkPhysicalDevice chosen_adapter = raw_adapters[0];
        for (VkPhysicalDevice adapter : raw_adapters) {
            VkPhysicalDeviceProperties properties = {};
            vkGetPhysicalDeviceProperties(adapter, &properties);

            if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                chosen_adapter = adapter;
                break;
            }
        }

        uint32_t family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(chosen_adapter, &family_count, nullptr);

        std::vector<VkQueueFamilyProperties> families(family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(chosen_adapter, &family_count, families.data());

        uint32_t chosen_queue = UINT32_MAX;
        for (uint32_t family = 0; family < family_count; family++) {
            auto queue = families[family];

            bool supportsCompute = (queue.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0;
            bool supportsTransfer = (queue.queueFlags & VK_QUEUE_TRANSFER_BIT) != 0;

            if (supportsCompute && supportsTransfer) {
                chosen_queue = family;
                break;
            }
        }

        if (chosen_queue == UINT32_MAX) {
            throw std::runtime_error("could not find suitable compute queue");
        }

        return Device(chosen_adapter, chosen_queue);
    }

    bool initialized() const noexcept {
        return m_instance != VK_NULL_HANDLE;
    }

private:
    VkInstance m_instance = VK_NULL_HANDLE;
};

int main() {
    Instance instance{};
    Device device = instance.requestDevice();

    return 0;
}