cmake_minimum_required(VERSION 3.20)
project(sandbox LANGUAGES CXX)

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

FetchContent_MakeAvailable(spdlog glaze flecs)

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
        flecs
        spdlog::spdlog
        glaze::glaze
)

# Precompiled Headers Configuration
target_precompile_headers(sandbox
        PRIVATE
        include/sandbox/pch.h
)