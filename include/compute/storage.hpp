#pragma once

#include <compute/pipeline.hpp>
#include <compute/reflection.hpp>
#include <compute/command.hpp>
#include <compute/device.hpp>
#include <compute/log.hpp>

#include <vector>
#include <memory>

#include <spdlog/spdlog.h>
#include <volk.h>

namespace Compute {
    class Device;
    class StorageBuffer;

    template<typename T>
    class HostBuffer;

    struct BufferReference {
        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
    };

    uint32_t findMemoryType(uint32_t typeBits, VkPhysicalDeviceMemoryProperties memProperties, VkMemoryPropertyFlags propertyFlags);

    /// Represents a pointer to the base of a mapped memory buffer.
    ///
    /// Upon destruction, this pointer automatically unmaps the memory.
    template<typename T>
    class MappedPtr {
    public:
        operator T*() {
            return mPtr;
        }

        operator const T*() const {
            return mPtr;
        }

        ~MappedPtr() {
            VkMemoryUnmapInfo unmapInfo = {};
            unmapInfo.sType = VK_STRUCTURE_TYPE_MEMORY_UNMAP_INFO;
            unmapInfo.memory = mBuffer->memory();

            LOG_VKRESULT(vkUnmapMemory2(mBuffer->device()->handle(), &unmapInfo), "Failed to unmap buffer memory");
        }

        /// The size of the underlying memory.
        VkDeviceSize size() {
            return mBuffer->size();
        }

    private:
        friend HostBuffer<T>;

        MappedPtr(std::shared_ptr<HostBuffer<T>> buffer) : mBuffer(std::move(buffer)) {
            VkMemoryMapInfo mapInfo = {};
            mapInfo.sType = VK_STRUCTURE_TYPE_MEMORY_MAP_INFO;
            mapInfo.size = mBuffer->size();
            mapInfo.memory = mBuffer->memory();
            mapInfo.offset = 0;

            LOG_VKRESULT(
                vkMapMemory2(mBuffer->device()->handle(), &mapInfo, &mPtr),
                "Failed to map memory to host buffer"
            );
        }

        std::shared_ptr<HostBuffer<T>> mBuffer;
        T* mPtr = nullptr;
    };

    template<typename T>
    class HostBuffer : public std::enable_shared_from_this<HostBuffer<T>> {
    public:
        ~HostBuffer() {
            freeMemory();
            destroyBuffer();
        }

        MappedPtr<T> map() {
            VkMemoryMapInfo mapInfo = {};
            mapInfo.sType = VK_STRUCTURE_TYPE_MEMORY_MAP_INFO;
            mapInfo.memory = mMemory;
            mapInfo.offset = 0;
            mapInfo.size = mSize;

            T* mappedPtr = nullptr;
            LOG_VKRESULT(
                vkMapMemory2(mDevice->handle(), &mapInfo, reinterpret_cast<void**>(&mappedPtr)),
                "Failed to map buffer to host memory"
            );

            return MappedPtr(mappedPtr, this->shared_from_this());
        }

        VkBuffer handle() {
            return mBuffer;
        }

        VkDeviceSize size() {
            return mSize;
        }

        VkDeviceMemory memory() {
            return mMemory;
        }

    private:
        friend Device;

        /// This constructor is private to ensure that a host buffer can only be constructed inside of
        /// a shared_ptr. This prevents buffer mapping breaking due to no shared pointers existing.
        HostBuffer(std::shared_ptr<Device> device, VkDeviceSize size)
            : mDevice(std::move(device)), mSize(size)
        {
            VkBufferCreateInfo bufferCi = {};
            bufferCi.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferCi.size = mSize;
            bufferCi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            bufferCi.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

            LOG_VKRESULT(
                vkCreateBuffer(mDevice->handle(), &bufferCi, nullptr, &mBuffer),
                "Failed to create staging buffer"
            );

            VkBufferMemoryRequirementsInfo2 memReqInfo = {};
            memReqInfo.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2;
            memReqInfo.buffer = mBuffer;

            VkMemoryRequirements2 memReqs = {};
            vkGetBufferMemoryRequirements2(mDevice->handle(), &memReqInfo, &memReqs);

            uint32_t memoryTypeIndex = findMemoryType(
                memReqs.memoryRequirements.memoryTypeBits,
                mDevice->memProperties().memoryProperties,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            );

            VkMemoryAllocateInfo memoryCi = {};
            memoryCi.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            memoryCi.memoryTypeIndex = memoryTypeIndex;
            memoryCi.allocationSize = memReqs.memoryRequirements.size;

            CHECK_VKRESULT(
                vkAllocateMemory(mDevice->handle(), &memoryCi, nullptr, &mMemory),
                "Failed to allocate staging buffer memory",
                destroyBuffer()
            );

            VkBindBufferMemoryInfo bindInfo = {};
            bindInfo.sType = VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO;
            bindInfo.buffer = mBuffer;
            bindInfo.memory = mMemory;
            bindInfo.memoryOffset = 0;

            CHECK_VKRESULT(
                vkBindBufferMemory2(mDevice->handle(), 1, &bindInfo),
                "Failed to bind staging buffer memory to buffer",
                {
                    freeMemory();
                    destroyBuffer();
                }
            );
        }

        void destroyBuffer() {
            if (mBuffer != VK_NULL_HANDLE) {
                vkDestroyBuffer(mDevice->handle(), mBuffer, nullptr);
                mBuffer = VK_NULL_HANDLE;
                spdlog::debug("Destroyed staging buffer");
            }
        }

        void freeMemory() {
            if (mMemory != VK_NULL_HANDLE) {
                vkFreeMemory(mDevice->handle(), mMemory, nullptr);
                mMemory = VK_NULL_HANDLE;
                spdlog::debug("Freed staging buffer memory");
            }
        }

        std::shared_ptr<Device> mDevice = nullptr;

        VkDeviceSize mSize = 0;
        VkBuffer mBuffer = VK_NULL_HANDLE;
        VkDeviceMemory mMemory = VK_NULL_HANDLE;
    };

    class StorageBuffer : public Resource {
    public:
        StorageBuffer(std::shared_ptr<Device> device, VkDeviceSize size);
        ~StorageBuffer();

        VkBuffer handle();

    private:
        void freeMemory();
        void destroyBuffer();

        std::shared_ptr<Device> mDevice = nullptr;

        VkDeviceSize mSize = 0;
        VkBuffer mBuffer = VK_NULL_HANDLE;
        VkDeviceMemory mMemory = VK_NULL_HANDLE;
    };
}