cmake_minimum_required(VERSION 3.20)
project(sandbox_library LANGUAGES CXX)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# =============================================================================
# 1. DEPENDENCIES
# =============================================================================
include(FetchContent)

FetchContent_Declare(glaze GIT_REPOSITORY https://github.com/stephenberry/glaze.git GIT_TAG v4.3.0 GIT_SHALLOW TRUE)
FetchContent_Declare(flecs GIT_REPOSITORY https://github.com/SanderMertens/flecs.git GIT_TAG v4.1.4 GIT_SHALLOW TRUE)
FetchContent_Declare(flatbuffers GIT_REPOSITORY https://github.com/google/flatbuffers.git GIT_TAG v23.5.26 GIT_SHALLOW TRUE)

set(FLECS_SHARED ON CACHE INTERNAL "Build Flecs as a shared library")
set(FLECS_STATIC OFF CACHE INTERNAL "Disable Flecs static library")
set(FLATBUFFERS_BUILD_TESTS OFF CACHE INTERNAL "Disable Flatbuffers tests")
set(FLATBUFFERS_BUILD_FLATC ON CACHE INTERNAL "Build flatc compiler")
set(glaze_ENABLE_AVX2 OFF CACHE INTERNAL "Disable AVX2 for Glaze")

FetchContent_MakeAvailable(glaze flecs flatbuffers)

# =============================================================================
# 2. CORE LIBRARY TARGET
# =============================================================================
add_library(sandbox_library SHARED
        source/core/engine.cpp
)

add_library(sandbox::library ALIAS sandbox_library)

target_compile_definitions(sandbox_library PRIVATE SANDBOX_BUILD_SHARED)
target_compile_features(sandbox_library PUBLIC cxx_std_23)

target_include_directories(sandbox_library
        PUBLIC  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/source
)

target_link_libraries(sandbox_library PUBLIC
        flecs
        glaze::glaze
        flatbuffers
)

# Apply the Launcher Precompiled Header
target_precompile_headers(sandbox_library
        PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/source/core/pch.h>"
)