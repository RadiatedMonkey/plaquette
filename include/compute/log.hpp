#pragma once

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
