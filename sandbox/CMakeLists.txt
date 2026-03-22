cmake_minimum_required(VERSION 3.20)

project(sandbox LANGUAGES CXX)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_DOCUMENTATION OFF CACHE BOOL "" FORCE)
set(BUILD_INSTALLER OFF CACHE BOOL "" FORCE)
set(BUILD_RTTR_DYNAMIC OFF CACHE BOOL "" FORCE)

include(FetchContent)

FetchContent_Declare(
        rttr
        GIT_REPOSITORY https://github.com/rttrorg/rttr.git
        GIT_TAG        v0.9.6
)

FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG        v1.15.0
)

FetchContent_Declare(
        glaze
        GIT_REPOSITORY https://github.com/stephenberry/glaze.git
        GIT_TAG        v4.3.0
        GIT_SHALLOW    TRUE
)

FetchContent_Declare(
        flecs
        GIT_REPOSITORY https://github.com/SanderMertens/flecs.git
        GIT_TAG        v4.1.4
)

FetchContent_MakeAvailable(rttr spdlog glaze flecs)

add_library(sandbox STATIC
    include/sandbox/sandbox.h
)

target_link_libraries(sandbox
        PRIVATE
        flecs
        spdlog::spdlog
        glaze::glaze
        rttr_core
)

target_include_directories(sandbox
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        PRIVATE "source"
)

add_library(sandbox::library ALIAS sandbox)