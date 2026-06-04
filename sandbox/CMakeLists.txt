cmake_minimum_required(VERSION 3.20)
project(sandbox LANGUAGES CXX)

set(CMAKE_POLICY_VERSION_MINIMUM "3.5")
# Crucial for static libraries being linked into a shared library
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(FetchContent)

FetchContent_Declare(spdlog GIT_REPOSITORY https://github.com/gabime/spdlog.git GIT_TAG v1.15.0 GIT_SHALLOW TRUE)
FetchContent_Declare(glaze GIT_REPOSITORY https://github.com/stephenberry/glaze.git GIT_TAG v4.3.0 GIT_SHALLOW TRUE)
FetchContent_Declare(flecs GIT_REPOSITORY https://github.com/SanderMertens/flecs.git GIT_TAG v4.1.4 GIT_SHALLOW TRUE)
FetchContent_Declare(physfs GIT_REPOSITORY https://github.com/icculus/physfs.git GIT_TAG release-3.2.0 GIT_SHALLOW TRUE)

# MARK: - Dependencies

# Flecs MUST be shared for dynamic plugins to share the ECS world state
set(FLECS_SHARED ON CACHE BOOL "" FORCE)
set(FLECS_STATIC OFF CACHE BOOL "" FORCE)

# Spdlog should be shared so plugins write to the same registry/thread pool
set(SPDLOG_BUILD_SHARED ON CACHE BOOL "" FORCE)

# PhysFS can safely remain static as it is usually heavily wrapped by your VFS
set(PHYSFS_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(PHYSFS_BUILD_STATIC ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(spdlog glaze flecs physfs)

# MARK: - Engine Configuration

# Building the engine as a SHARED library (or change to STATIC if desired)
add_library(sandbox SHARED
        source/core/properties.cpp
        source/core/engine.cpp
        source/utilities/filesystem.cpp
        source/core/plugin.cpp
        source/subsystems/logger/logger.cpp
        source/subsystems/runner/runner.cpp
        source/subsystems/filesystem/filesystem.cpp
        source/utilities/loader.cpp
        include/sandbox/sandbox.h
)
add_library(sandbox::library ALIAS sandbox)

target_compile_definitions(sandbox PRIVATE SANDBOX_BUILD_SHARED)
target_compile_features(sandbox PUBLIC cxx_std_23)

target_include_directories(sandbox
        PUBLIC  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/source
)

# Link Flecs as a shared target again
target_link_libraries(sandbox PUBLIC flecs spdlog::spdlog glaze::glaze physfs-static)

target_precompile_headers(sandbox PRIVATE include/sandbox/pch.h)

set_target_properties(sandbox PROPERTIES
        BUILD_RPATH "\$ORIGIN"
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH "\$ORIGIN"
)