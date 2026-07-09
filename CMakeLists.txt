cmake_minimum_required(VERSION 3.20)
project(rt)

set(CMAKE_CXX_STANDARD 20)
set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)

set(SOURCES
    src/main.cpp
)


add_subdirectory(vendor)
add_subdirectory(vendor/volk)

add_executable(rt ${SOURCES})
target_include_directories(rt PRIVATE include)
target_link_libraries(rt PRIVATE volk)
