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
        include/sandbox/pch.h

        # CORE
        include/sandbox/core/sandbox_fwd.h
        include/sandbox/core/extension.h
        source/core/extension.cpp
        include/sandbox/core/engine.h
        include/sandbox/core/engine.inl
        source/core/engine.cpp
        include/sandbox/core/properties.h
        include/sandbox/core/properties.inl
        source/core/properties.cpp

        # ECS (Automated Flecs Wrappers)
        include/sandbox/ecs/systems_ext.h
        include/sandbox/ecs/systems_ext.inl
        include/sandbox/ecs/systems_evt.h
        source/ecs/systems_ext.cpp
        include/sandbox/ecs/triggers_ext.h
        include/sandbox/ecs/triggers_ext.inl
        include/sandbox/ecs/triggers_evt.h
        source/ecs/triggers_ext.cpp
        include/sandbox/ecs/events_ext.h
        include/sandbox/ecs/events_ext.inl
        include/sandbox/ecs/events_evt.h
        source/ecs/events_ext.cpp
        include/sandbox/ecs/stages_ext.h
        include/sandbox/ecs/stages_evt.h
        source/ecs/stages_ext.cpp
        include/sandbox/ecs/scopes_ext.h
        include/sandbox/ecs/scopes_evt.h
        source/ecs/scopes_ext.cpp

        # DATA (Memory & Reflection)
        include/sandbox/data/caches_ext.h
        include/sandbox/data/caches_ext.inl
        include/sandbox/data/caches_evt.h
        source/data/caches_ext.cpp
        include/sandbox/data/storage_ext.h
        include/sandbox/data/storage_ext.inl
        include/sandbox/data/storage_evt.h
        source/data/storage_ext.cpp
        include/sandbox/data/registry.h
        include/sandbox/data/registry.inl
        source/data/registry.cpp
        include/sandbox/data/registration.h

        # IO (Files & Serialization)
        include/sandbox/io/filesystem_ext.h
        include/sandbox/io/filesystem_evt.h
        source/io/filesystem_ext.cpp
        include/sandbox/io/serializer_ext.h
        include/sandbox/io/serializer_ext.inl
        include/sandbox/io/serializer_evt.h
        source/io/serializer_ext.cpp

        # SYSTEM (Engine Services)
        include/sandbox/system/clock_ext.h
        include/sandbox/system/clock_evt.h
        source/system/clock_ext.cpp
        include/sandbox/system/dependencies_ext.h
        include/sandbox/system/dependencies_evt.h
        source/system/dependencies_ext.cpp

        # DIAGNOSTICS
        include/sandbox/diagnostics/logger.h
        include/sandbox/diagnostics/logger.inl
        source/diagnostics/logger.cpp
        include/sandbox/diagnostics/logger_ext.h
        include/sandbox/diagnostics/logger_evt.h
        include/sandbox/diagnostics/scoped_logger.h
        include/sandbox/diagnostics/scoped_logger.inl
        source/diagnostics/scoped_logger.cpp
        include/sandbox/diagnostics/diagnostics_ext.h
        include/sandbox/diagnostics/diagnostics_evt.h
        source/diagnostics/diagnostics_ext.cpp
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