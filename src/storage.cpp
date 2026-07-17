#include <compute/storage.hpp>
#include <compute/device.hpp>

#include <spdlog/spdlog.h>
#include <volk.h>

namespace Compute {
    StorageBuffer::StorageBuffer(std::shared_ptr<Device> device, VkDeviceSize size) 
        : mDevice(std::move(device)), mSize(size)
    {
        VkBufferCreateInfo bufferCi = {};
        bufferCi.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCi.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        bufferCi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferCi.size = size;

        VkResult result = vkCreateBuffer(mDevice->handle(), &bufferCi, nullptr, &mBuffer);
        if (result != VK_SUCCESS) {
            spdlog::error("Failed to create storage buffer: {}", static_cast<uint32_t>(result));
            throw std::runtime_error("Failed to create storage buffer");
        }
        spdlog::debug("Created storage buffer");

        uint32_t memTypeIndex = UINT32_MAX;
        VkPhysicalDeviceMemoryProperties memProperties = mDevice->memProperties();
        for(uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            VkMemoryType memType = memProperties.memoryTypes[i];
            
            if ((memType.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0) {
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

        result = vkAllocateMemory(mDevice->handle(), &memoryCi, nullptr, &mMemory);
        if (result != VK_SUCCESS) {
            destroyBuffer();

            spdlog::error("Failed to allocate device local memory");
            throw std::runtime_error("Failed to allocate device local memory");
        }

        spdlog::debug("Allocated {} bytes of storage buffer memory", mSize);
    }

    StorageBuffer::StorageBuffer(std::shared_ptr<Device> device, const std::vector<float>& data) : StorageBuffer(mDevice, data.size() * sizeof(float))
    {
        // Buffer already created, just need to copy over the data.

        
    }

    StorageBuffer::~StorageBuffer() {
        destroyBuffer();

        if (mMemory != VK_NULL_HANDLE) {
            vkFreeMemory(mDevice->handle(), mMemory, nullptr);
            mMemory = VK_NULL_HANDLE;

            spdlog::debug("Freed storage buffer memory");
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