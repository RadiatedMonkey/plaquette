#include <cassert>
#include <memory>
#include <vector>
#include <iostream>

#include <GLFW/glfw3.h>
#include <volk.h>

static constexpr int WINDOW_WIDTH = 800;
static constexpr int WINDOW_HEIGHT = 600;
static constexpr const char* WINDOW_TITLE = "Relativistic Raytracer";

static constexpr const char* ENABLED_INSTANCE_EXTENSIONS[] = {
    "VK_KHR_surface",
    "VK_KHR_win32_surface"
};

static constexpr const char* ENABLED_INSTANCE_LAYERS[] = {
    "VK_LAYER_KHRONOS_validation"
};

int main() {
    int glfwResult = glfwInit();
    assert(glfwResult == GLFW_TRUE);

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, nullptr, nullptr);
    assert(window != nullptr);

    VkResult result = volkInitialize();
    assert(result == VK_SUCCESS);

    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = WINDOW_TITLE;
    app_info.apiVersion = VK_API_VERSION_1_4;

    VkInstanceCreateInfo instance_info = {};
    instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_info.pApplicationInfo = &app_info;
    instance_info.enabledExtensionCount = std::size(ENABLED_INSTANCE_EXTENSIONS);
    instance_info.ppEnabledExtensionNames = ENABLED_INSTANCE_EXTENSIONS;
    instance_info.enabledLayerCount = std::size(ENABLED_INSTANCE_LAYERS);
    instance_info.ppEnabledLayerNames = ENABLED_INSTANCE_LAYERS;

    VkInstance instance = VK_NULL_HANDLE;
    result = vkCreateInstance(&instance_info, nullptr, &instance);
    assert(result == VK_SUCCESS);

    std::cout << "Created instance" << std::endl;

    volkLoadInstance(instance);

    uint32_t adapter_count = 0;
    result = vkEnumeratePhysicalDevices(instance, &adapter_count, nullptr);
    assert(result == VK_SUCCESS);

    auto adapters = std::vector<VkPhysicalDevice>(adapter_count);
    result = vkEnumeratePhysicalDevices(instance, &adapter_count, adapters.data());
    assert(result == VK_SUCCESS);

    for (const VkPhysicalDevice& adapter : adapters) {
        VkPhysicalDeviceProperties properties = {};
        vkGetPhysicalDeviceProperties(adapter, &properties);

        std::cout << "Adapter name is: " << properties.deviceName << std::endl;
    }

    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}