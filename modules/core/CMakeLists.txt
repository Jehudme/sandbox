cmake_minimum_required(VERSION 3.20)
project(sandbox_core_plugin VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
set(SOURCES
    source/configuration/configuration.cpp
    source/filesystem/filesystem.cpp
    source/logs/logger.cpp
    source/runtime/runtime.cpp
)

# Headers
set(HEADERS
    source/configuration/configuration.h
    source/filesystem/filesystem.h
    source/logs/logger.h
    source/runtime/runtime.h
    include/sandbox/abi/configuration.h
    include/sandbox/abi/filesystem.h
    include/sandbox/abi/logs.h
    include/sandbox/abi/runtime.h
    include/sandbox/sdk/configuration.hpp
    include/sandbox/sdk/filesystem.hpp
    include/sandbox/sdk/logs.hpp
    include/sandbox/sdk/runtime.hpp
)

# Create shared library
option(SANDBOX_MODULES_STATIC "Build modules statically and link them directly" OFF)

if(SANDBOX_MODULES_STATIC)
    add_library(sandbox_plugin STATIC ${SOURCES} ${HEADERS})
else()
    add_library(sandbox_plugin MODULE ${SOURCES} ${HEADERS})
    set_target_properties(sandbox_plugin PROPERTIES
        PREFIX ""
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
    )
endif()

add_library(sandbox::plugins ALIAS sandbox_plugin)

# Includes
target_include_directories(sandbox_plugin PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/source
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/sandbox/include
)

target_include_directories(sandbox_plugin PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

include(FetchContent)

set(SANDBOX_ORIGINAL_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})
set(BUILD_SHARED_LIBS OFF)

find_package(spdlog QUIET)
if(NOT spdlog_FOUND)
    FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.15.3
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(spdlog)
endif()

find_package(miniz QUIET)
if(NOT miniz_FOUND)
    FetchContent_Declare(
        miniz
        GIT_REPOSITORY https://github.com/richgel999/miniz.git
        GIT_TAG 3.0.2
        GIT_SHALLOW TRUE
    )
    # Force new CMake versions (3.30+) to accept miniz's legacy cmake_minimum_required(3.0)
    set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "" FORCE)
    FetchContent_MakeAvailable(miniz)
endif()

set(BUILD_SHARED_LIBS ${SANDBOX_ORIGINAL_BUILD_SHARED_LIBS})

# Link against core engine and deps
target_link_libraries(sandbox_plugin PRIVATE
    flecs_headers
    spdlog::spdlog
    miniz
)

# Copy to bin directory
add_custom_command(TARGET sandbox_plugin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    $<TARGET_FILE:sandbox_plugin>
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<TARGET_FILE_NAME:sandbox_plugin>
)

target_precompile_headers(sandbox_plugin PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_SOURCE_DIR}/sandbox/source/pch.h>")

# =============================================================================
# INSTALLATION
# =============================================================================
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(GNUInstallDirs)
    install(TARGETS sandbox_plugin
        EXPORT sandboxTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
endif()
