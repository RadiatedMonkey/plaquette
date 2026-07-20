cmake_minimum_required(VERSION 3.20)
project(rt)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)

set(SOURCES
    src/main.cpp
    src/instance.cpp
    src/device.cpp
    src/pipeline.cpp
    src/storage.cpp
    src/reflection.cpp
    src/command.cpp
    src/simulation.cpp
    
    src/spirv_reflect.c
)

add_subdirectory(vendor/spdlog)
add_subdirectory(vendor/glfw)
add_subdirectory(vendor/volk)

message(${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_executable(rt ${SOURCES})
target_include_directories(rt PRIVATE include)
target_compile_definitions(rt PRIVATE BUILD_DIR="${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
target_link_libraries(rt PRIVATE volk glfw spdlog)
