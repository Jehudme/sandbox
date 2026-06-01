cmake_minimum_required(VERSION 3.20)
project(sandbox LANGUAGES CXX)

set(CMAKE_POLICY_VERSION_MINIMUM "3.5")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(FetchContent)

FetchContent_Declare(spdlog GIT_REPOSITORY https://github.com/gabime/spdlog.git GIT_TAG v1.15.0 GIT_SHALLOW TRUE)
FetchContent_Declare(glaze GIT_REPOSITORY https://github.com/stephenberry/glaze.git GIT_TAG v4.3.0 GIT_SHALLOW TRUE)
FetchContent_Declare(flecs GIT_REPOSITORY https://github.com/SanderMertens/flecs.git GIT_TAG v4.1.4 GIT_SHALLOW TRUE)
FetchContent_Declare(physfs GIT_REPOSITORY https://github.com/icculus/physfs.git GIT_TAG release-3.2.0 GIT_SHALLOW TRUE)

# Must be shared to prevent ECS ID collisions between plugins and the host
set(FLECS_SHARED ON CACHE BOOL "" FORCE)
set(FLECS_STATIC OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(spdlog glaze flecs physfs)

# Build the engine as a SHARED library
add_library(sandbox SHARED
        source/core/properties.cpp
        source/core/engine.cpp
        source/utilities/filesystem.cpp
        source/core/plugin.cpp
        source/modules/logger.cpp
        source/modules/runner.cpp
        source/modules/vfs.cpp
        source/exceptions/vfs_exceptions.cpp
        source/modules/plugins.cpp
        include/sandbox/core/arguments.h
)
add_library(sandbox::library ALIAS sandbox)

target_compile_definitions(sandbox PRIVATE SANDBOX_BUILD_SHARED)
target_compile_features(sandbox PUBLIC cxx_std_23)

target_include_directories(sandbox
        PUBLIC  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/source
)

# Link Flecs as a shared target (removed _static)
target_link_libraries(sandbox PUBLIC flecs spdlog::spdlog glaze::glaze physfs-static)

target_precompile_headers(sandbox PRIVATE include/sandbox/pch.h)

set_target_properties(sandbox PROPERTIES
        BUILD_RPATH "\$ORIGIN"
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH "\$ORIGIN"
)