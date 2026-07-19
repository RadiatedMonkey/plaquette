#include <compute/storage.hpp>
#include <compute/device.hpp>

#include <spdlog/spdlog.h>
#include <volk.h>

namespace Compute {
    StorageBuffer::StorageBuffer(std::shared_ptr<Device> device, VkDeviceSize size) 
        : mDevice(std::move(device)), mSize(size) 
    {
        auto deviceBuffer = createBoundBuffer(size, false);
        mBuffer = deviceBuffer.buffer;
        mMemory = deviceBuffer.memory;
    }
    
    StorageBuffer::~StorageBuffer() {
        freeMemory();
        destroyBuffer();
    }

    uint32_t findMemoryType(uint32_t typeBits, VkPhysicalDeviceMemoryProperties memProperties, VkMemoryPropertyFlags propertyFlags) {
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            bool hardwareSupported = typeBits & (1 << i);
            bool hasFlags = (memProperties.memoryTypes[i].propertyFlags & propertyFlags) == propertyFlags;

            if (hardwareSupported && hasFlags) {
                return i;
            }
        }

        return UINT32_MAX;
    }

    BufferReference StorageBuffer::createBoundBuffer(VkDeviceSize size, bool isTransferSrc) {
        BufferReference bufferRef = {};

        VkBufferCreateInfo bufferCi = {};
        bufferCi.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferCi.size = size;

        if (isTransferSrc) {
            bufferCi.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        } else {
            bufferCi.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        }

        VkResult result = vkCreateBuffer(mDevice->handle(), &bufferCi, nullptr, &bufferRef.buffer);
        if (result != VK_SUCCESS) {
            spdlog::error("Failed to create storage buffer of size {}: {}", size, static_cast<uint32_t>(size));
            throw std::runtime_error("Failed to create storage buffer");
        }

        VkPhysicalDeviceMemoryProperties memProperties = mDevice->memProperties().memoryProperties;

        VkBufferMemoryRequirementsInfo2 memRequirementsInfo = {};
        memRequirementsInfo.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2;
        memRequirementsInfo.buffer = bufferRef.buffer;

        VkMemoryRequirements2 memRequirements = {};
        memRequirements.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;

        vkGetBufferMemoryRequirements2(mDevice->handle(), &memRequirementsInfo, &memRequirements);

        VkMemoryPropertyFlags memFlags;
        if (!isTransferSrc) {
            memFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        } else {
            memFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        }

        uint32_t memTypeIndex = findMemoryType(memRequirements.memoryRequirements.memoryTypeBits, memProperties, memFlags);
        if (memTypeIndex == UINT32_MAX) { // Not found
            destroyBuffer();

            spdlog::error("Failed to find suitable memory type for buffer");
            throw std::runtime_error("Failed to find suitable memory type for buffer");
        }

        VkMemoryAllocateInfo memoryCi = {};
        memoryCi.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memoryCi.allocationSize = memRequirements.memoryRequirements.size;
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
            destroyBuffer();
            freeMemory();

            spdlog::error("Failed to bind allocated memory to storage buffer: {}", static_cast<uint32_t>(result));
            throw std::runtime_error("Failed to bind allocated memory to storage buffer");
        }

        return bufferRef;
    }

    void StorageBuffer::destroyBuffer() {
        if (mBuffer != VK_NULL_HANDLE) {
            vkDestroyBuffer(mDevice->handle(), mBuffer, nullptr);
            mBuffer = VK_NULL_HANDLE;

            spdlog::debug("Destroyed storage buffer");
        }
    }

    void StorageBuffer::freeMemory() {
        if (mMemory != VK_NULL_HANDLE) {
            vkFreeMemory(mDevice->handle(), mMemory, nullptr);
            mMemory = VK_NULL_HANDLE;

            spdlog::debug("Freed storage buffer memory");
        }
    }
}