#pragma once

#include <type_traits>

#define LOG_VKRESULT(x, msg) do {                                               \
    VkResult vkresultCheck = (x);                                               \
    if (vkresultCheck != VK_SUCCESS) {                                          \
        spdlog::error(msg ": {}", static_cast<uint32_t>(vkresultCheck));        \
        throw std::runtime_error(msg);                                          \
    }                                                                           \
} while (false)

#define CHECK_VKRESULT(x, msg, cleanup) do {                                    \
    VkResult vkresultCheck = (x);                                               \
    if (vkresultCheck != VK_SUCCESS) {                                          \
        do {                                                                    \
            cleanup;                                                            \
        } while (false);                                                        \
                                                                                \
        spdlog::error(msg ": {}", static_cast<uint32_t>(vkresultCheck));        \
        throw std::runtime_error(msg);                                          \
    }                                                                           \
} while (false)

namespace Plaq {
    template<typename F>
    concept RealFloat = std::is_same_v<F, float> || std::is_same_v<F, double>;
}