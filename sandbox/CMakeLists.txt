cmake_minimum_required(VERSION 3.20)
project(sandbox LANGUAGES CXX)

include(FetchContent)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(BUILD_EXAMPLES      OFF CACHE BOOL "" FORCE)
set(BUILD_UNIT_TESTS    OFF CACHE BOOL "" FORCE)
set(BUILD_DOCUMENTATION OFF CACHE BOOL "" FORCE)
set(BUILD_INSTALLER     OFF CACHE BOOL "" FORCE)

# Prefer static RTTR
set(RTTR_BUILD_STATIC ON  CACHE BOOL "" FORCE)
set(RTTR_BUILD_SHARED OFF CACHE BOOL "" FORCE)

# Avoid install/packaging when embedded
set(RTTR_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)

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
        include/sandbox/utilities/properties.h
        include/sandbox/utilities/properties.inl
        source/utilities/properties.cpp
        source/core/logger.cpp
        include/sandbox/core/logger.h
        include/sandbox/core/logger.inl
        source/core/scoped_logger.cpp
        include/sandbox/core/scoped_logger.h
        include/sandbox/core/scoped_logger.inl
        include/sandbox/core/registry.cpp
        include/sandbox/core/registry.h
        include/sandbox/core/registry.inl
        include/sandbox/core/registration.h
)

target_include_directories(sandbox
        PUBLIC  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/source
)

target_link_libraries(sandbox
        PUBLIC
        flecs_static
        spdlog::spdlog
        glaze::glaze
        RTTR::Core
)

add_library(sandbox::library ALIAS sandbox)