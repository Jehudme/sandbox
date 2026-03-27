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
        include/sandbox/core/extension.h
        source/core/extension.cpp
        include/sandbox/core/engine.h
        include/sandbox/core/engine.inl
        source/core/engine.cpp
        include/sandbox/core/properties.h
        include/sandbox/core/properties.inl
        source/core/properties.cpp

        # ECS (Automated Flecs Wrappers)
        include/sandbox/ecs/systems.h
        include/sandbox/ecs/systems.inl
        source/ecs/systems.cpp
        include/sandbox/ecs/triggers.h
        include/sandbox/ecs/triggers.inl
        source/ecs/triggers.cpp
        include/sandbox/ecs/events.h
        include/sandbox/ecs/events.inl
        source/ecs/events.cpp
        include/sandbox/ecs/stages.h
        source/ecs/stages.cpp
        include/sandbox/ecs/scopes.h
        source/ecs/scopes.cpp

        # DATA (Memory & Reflection)
        include/sandbox/data/caches.h
        include/sandbox/data/caches.inl
        source/data/caches.cpp
        include/sandbox/data/storage.h
        include/sandbox/data/storage.inl
        source/data/storage.cpp
        include/sandbox/data/registry.h
        include/sandbox/data/registry.inl
        source/data/registry.cpp
        include/sandbox/data/registration.h

        # IO (Files & Serialization)
        include/sandbox/io/filesystem.h
        source/io/filesystem.cpp
        include/sandbox/io/serializer.h
        include/sandbox/io/serializer.inl
        source/io/serializer.cpp

        # SYSTEM (Engine Services)
        include/sandbox/system/clock.h
        source/system/clock.cpp
        include/sandbox/system/dependencies.h
        source/system/dependencies.cpp

        # DIAGNOSTICS
        include/sandbox/diagnostics/logger.h
        include/sandbox/diagnostics/logger.inl
        source/diagnostics/logger.cpp
        include/sandbox/diagnostics/scoped_logger.h
        include/sandbox/diagnostics/scoped_logger.inl
        source/diagnostics/scoped_logger.cpp
        include/sandbox/diagnostics/diagnostics.h
        source/diagnostics/diagnostics.cpp
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