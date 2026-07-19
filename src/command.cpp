#include <compute/command.hpp>
#include <compute/device.hpp>
#include <utility>

#include <volk.h>
#include <spdlog/spdlog.h>

namespace Compute {
    VkCommandBuffer Commands::handle() {
        return mBuffer;
    }

    Commands::Commands(std::shared_ptr<Device> device, VkCommandBuffer buffer)
        : mDevice(std::move(device)), mBuffer(buffer) {}

    Commands::Commands(Commands&& other) noexcept : mDevice(std::move(other.mDevice)), mBuffer(other.mBuffer) {
        other.mBuffer = VK_NULL_HANDLE;
    }

    Commands::~Commands() {
        if (mBuffer != VK_NULL_HANDLE) {
            vkFreeCommandBuffers(mDevice->handle(), mDevice->cmdPool(), 1, &mBuffer);
            mBuffer = VK_NULL_HANDLE;

            spdlog::debug("Freed command buffer");
        }
    }

    void Commands::reset() {
        VkResult result = vkResetCommandBuffer(mBuffer, 0);
        if (result != VK_SUCCESS) {
            spdlog::error("Failed to reset command buffer: {}", static_cast<uint32_t>(result));
            throw std::runtime_error("Failed to reset command buffer");
        }
    }
}