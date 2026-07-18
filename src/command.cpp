#include <compute/command.hpp>
#include <compute/device.hpp>

#include <volk.h>
#include <spdlog/spdlog.h>

namespace Compute {
    Queue::Queue(std::shared_ptr<Device> device, VkQueue queue, uint32_t queueFamilyIndex) 
        : mDevice(device), mQueue(queue), mQueueFamilyIndex(queueFamilyIndex) 
    {
        VkCommandPoolCreateInfo poolCi = {};
        poolCi.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolCi.queueFamilyIndex = queueFamilyIndex;
        
        VkResult result = vkCreateCommandPool(mDevice->handle(), &poolCi, nullptr, &mPool);
        if (result != VK_SUCCESS) {
            spdlog::error("Failed to create command pool: {}", static_cast<uint32_t>(result));
            throw std::runtime_error("Failed to create command pool");
        }
    }

    Queue::~Queue() {
        destroyCommandPool();
    }

    Commands Queue::record() {
        VkCommandBufferAllocateInfo bufferCi = {};
        bufferCi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        bufferCi.commandPool = mPool;
        bufferCi.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        VkCommandBuffer buffer = VK_NULL_HANDLE;
        VkResult result = vkAllocateCommandBuffers(mDevice->handle(), &bufferCi, &buffer);
        if (result != VK_SUCCESS) {
            spdlog::error("Failed to allocate command buffer: {}", static_cast<uint32_t>(result));
            throw std::runtime_error("Failed to allocate command buffer");
        }

        return Commands(mDevice, buffer);
    }

    VkCommandPool Queue::pool() {
        return mPool;
    }

    void Queue::destroyCommandPool() {
        if (mPool != VK_NULL_HANDLE) {
            vkDestroyCommandPool(mDevice->handle(), mPool, nullptr);
            mPool = VK_NULL_HANDLE;

            spdlog::debug("Destroyed command pool");
        }
    }

    Commands::Commands(std::shared_ptr<Device> device, VkCommandBuffer buffer)
        : mDevice(device), mBuffer(buffer) {}

    Commands::~Commands() {
        if (mBuffer != VK_NULL_HANDLE) {
            vkFreeCommandBuffers(mDevice->handle(), mDevice->queue().pool(), 1, &mBuffer);
            mBuffer = VK_NULL_HANDLE;

            spdlog::debug("Freed command buffer");
        }
    }
}