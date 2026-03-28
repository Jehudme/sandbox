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
        source/utilities/properties.cpp
        include/sandbox/utilities/properties.h
        include/sandbox/utilities/properties.inl
        source/core/extension.cpp
        include/sandbox/core/extension.h
        source/core/type_registry.cpp
        include/sandbox/core/type_registry.h
        include/sandbox/core/type_registry.inl
        include/sandbox/core/type_registration.h
        source/utilities/logger.cpp
        include/sandbox/extensions/logger.h
        include/sandbox/extensions/events.h
        source/extensions/logger.cpp
        include/sandbox/extensions/events.inl
        include/sandbox/extensions/logger.inl
)

target_compile_features(sandbox PUBLIC cxx_std_23)

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

target_precompile_headers(sandbox
        PRIVATE
        "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/include/sandbox/pch.h>"
)

add_library(sandbox::library ALIAS sandbox)