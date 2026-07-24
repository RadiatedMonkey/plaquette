cmake_minimum_required(VERSION 3.20)
project(rt)

include(FetchContent)

FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog
  GIT_TAG        v1.17.0
)

FetchContent_Declare(
  slang
  GIT_REPOSITORY https://github.com/shader-slang/slang
  GIT_TAG        v2026.14
)

FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw
  GIT_BRANCH     master                         # GLFW has not created a new release in several years
)

FetchContent_Declare(
  volk
  GIT_REPOSITORY https://github.com/zeux/volk
  GIT_TAG        1.4.350
)

set(SLANG_ENABLE_SLANGC OFF CACHE BOOL "" FORCE)
set(SLANG_ENABLE_GFX    OFF CACHE BOOL "" FORCE)
set(SLANG_ENABLE_TESTS  OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(spdlog slang glfw volk)

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
    src/fence.cpp

    src/workloads/rand.cpp
    src/workloads/mean.cpp

    src/spirv_reflect.c
)

message(${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_executable(rt ${SOURCES})
target_include_directories(rt PRIVATE include)
target_compile_definitions(rt PRIVATE BUILD_DIR="${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
target_link_libraries(rt PRIVATE volk glfw spdlog slang)
