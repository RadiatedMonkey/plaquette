#include <compute/storage.hpp>
#include <compute/device.hpp>

#include <spdlog/spdlog.h>
#include <volk.h>

namespace Compute {
    StorageBuffer::StorageBuffer(std::shared_ptr<Device> device, VkDeviceSize size) 
        : mDevice(std::move(device)), mSize(size) 
    {
        auto deviceBuffer = createBoundBuffer(size, true);
        mBuffer = deviceBuffer.buffer;
        mMemory = deviceBuffer.memory;
    }
    
    StorageBuffer::~StorageBuffer() {
        if (mMemory != VK_NULL_HANDLE) {
            vkFreeMemory(mDevice->handle(), mMemory, nullptr);
            mMemory = VK_NULL_HANDLE;

            spdlog::debug("Freed storage buffer memory");
        }

        destroyBuffer();
    }

    BufferReference StorageBuffer::createBoundBuffer(VkDeviceSize size, bool deviceLocal) {
        BufferReference bufferRef = {};

        VkBufferCreateInfo bufferCi = {};
        bufferCi.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferCi.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        bufferCi.size = size;

        VkResult result = vkCreateBuffer(mDevice->handle(), &bufferCi, nullptr, &bufferRef.buffer);
        if (result != VK_SUCCESS) {
            spdlog::error("Failed to create storage buffer of size {}: {}", size, static_cast<uint32_t>(size));
            throw std::runtime_error("Failed to create storage buffer");
        }

        uint32_t memTypeIndex = UINT32_MAX;

        VkPhysicalDeviceMemoryProperties memProperties = mDevice->memProperties().memoryProperties;
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((memProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0) {
                memTypeIndex = i;
                break;
            }
        }

        if (memTypeIndex == UINT32_MAX) {
            destroyBuffer();

            spdlog::error("Failed to find device local memory type");
            throw std::runtime_error("Failed to find device local memory type");
        }

        VkMemoryAllocateInfo memoryCi = {};
        memoryCi.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memoryCi.allocationSize = size;
        memoryCi.memoryTypeIndex = memTypeIndex;

        result = vkAllocateMemory(mDevice->handle(), &memoryCi, nullptr, &bufferRef.memory);
        if (result != VK_SUCCESS) {
            destroyBuffer();

            spdlog::error("Failed to allocate {} bytes of device local memory: {}", size, static_cast<uint32_t>(result));
            throw std::runtime_error("Failed to allocate device local memory");
        }

        VkBindBufferMemoryInfo bindInfo = {};
        bindInfo.sType = VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO;
        bindInfo.buffer = bufferRef.buffer;
        bindInfo.memory = bufferRef.memory;
        bindInfo.memoryOffset = 0;
        
        result = vkBindBufferMemory2(mDevice->handle(), 1, &bindInfo);
        if (result != VK_SUCCESS) {
            spdlog::error("Failed to bind allocated memory to storage buffer: {}", static_cast<uint32_t>(result));
            throw std::runtime_error("Failed to bind allocated memory to storage buffer");
        }
    }

    void StorageBuffer::destroyBuffer() {
        if (mBuffer != VK_NULL_HANDLE) {
            vkDestroyBuffer(mDevice->handle(), mBuffer, nullptr);
            mBuffer = VK_NULL_HANDLE;

            spdlog::debug("Destroyed storage buffer");
        }
    }
}