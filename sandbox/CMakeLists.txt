cmake_minimum_required(VERSION 3.20)
project(sandbox LANGUAGES CXX)

# FIX: Forces modern CMake to bypass the archaic 2.8 compatibility block
# in third-party libraries like PhysFS by setting a safety minimum policy layer.
set(CMAKE_POLICY_VERSION_MINIMUM "3.5")

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(FetchContent)

# -----------------------------------------------------------------------------
# Third-Party Dependencies
# -----------------------------------------------------------------------------

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

FetchContent_Declare(
        physfs
        GIT_REPOSITORY https://github.com/icculus/physfs.git
        GIT_TAG        release-3.2.0
        GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(spdlog glaze flecs physfs)

# -----------------------------------------------------------------------------
# Sandbox Library Target
# -----------------------------------------------------------------------------

add_library(sandbox STATIC
        source/core/properties.cpp
        source/core/engine.cpp

        include/sandbox/utilities/properties.h
        include/sandbox/core/engine.h
        include/sandbox/core/ecs.h
        include/sandbox/core/plugin.h
        include/sandbox/utilities/properties.inl
        include/sandbox/utilities/filesystem.h
        source/utilities/filesystem.cpp
        include/sandbox/core/platform.h
        source/core/plugin.cpp
        include/sandbox/utilities/events.h
        include/sandbox/utilities/events.inl
        source/modules/logger.h
        source/modules/logger.cpp
        include/sandbox/events/logger.h
        include/sandbox/events/logger.inl
        include/sandbox/macros/logger.h
        source/modules/runner.cpp
        source/modules/runner.h
        include/sandbox/macros/runner.h
        include/sandbox/events/vfs.h
        source/modules/vfs.cpp
        source/modules/vfs.h
        include/sandbox/macros/vfs.h
        include/sandbox/exceptions/vfs_exceptions.h
        source/exceptions/vfs_exceptions.cpp
)

add_library(sandbox::library ALIAS sandbox)

# Compile Features & Standards
target_compile_features(sandbox PUBLIC cxx_std_23)

# Include Directories
target_include_directories(sandbox
        PUBLIC  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/source
)

# Target Linkage
target_link_libraries(sandbox
        PUBLIC
        flecs_static
        spdlog::spdlog
        glaze::glaze
        physfs-static
)

# Precompiled Headers Configuration
target_precompile_headers(sandbox
        PRIVATE
        include/sandbox/pch.h
)