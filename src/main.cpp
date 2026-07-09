#include <cassert>

#include <volk.h>

static constexpr const char* ENABLED_INSTANCE_EXTENSIONS[] = {

};

static constexpr const char* ENABLED_INSTANCE_LAYERS[] = {
    "VK_LAYER_KHRONOS_validation"
};

int main() {
    VkResult result = volkInitialize();
    assert(result == VK_SUCCESS);

    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "Relativistic Raytracer";
    app_info.apiVersion = VK_API_VERSION_1_4;

    VkInstanceCreateInfo instance_info = {};
    instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_info.pApplicationInfo = &app_info;
    instance_info.

    return 0;
}