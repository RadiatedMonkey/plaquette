#include <plaquette/command.hpp>
#include <plaquette/device.hpp>
#include <plaquette/util.hpp>

#include <utility>

#include <volk.h>
#include <spdlog/spdlog.h>

namespace Plaq {
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
        assert(mBuffer != VK_NULL_HANDLE);

        LOG_VKRESULT(
            vkResetCommandBuffer(mBuffer, 0),
            "Failed to reset command buffer"
        );
    }
}