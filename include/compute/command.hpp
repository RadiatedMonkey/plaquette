#pragma once

#include <memory>

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

namespace Compute {
    class Device;

    class Commands {
    public:
        ~Commands();
        
        void finish();
        void submit();

    private:
        friend class Queue;

        Commands(std::shared_ptr<Device> device, VkCommandBuffer buffer);

        std::shared_ptr<Device> mDevice = nullptr;

        VkCommandBuffer mBuffer = VK_NULL_HANDLE;
    };

    class Queue {
    public:
        ~Queue();

        VkCommandPool pool();
        Commands record();

    private:
        friend Device;

        Queue() = default;
        Queue(std::shared_ptr<Device> device, VkQueue queue, uint32_t queueFamilyIndex);    

        void destroyCommandPool();

        VkCommandPool mPool = VK_NULL_HANDLE;
        std::shared_ptr<Device> mDevice = nullptr;
        VkQueue mQueue = VK_NULL_HANDLE;

        uint32_t mQueueFamilyIndex = UINT32_MAX;
    };
}