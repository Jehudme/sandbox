cmake_minimum_required(VERSION 3.20)

# Rename the root project so it doesn't collide with the inner library project
project(sandbox_root LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_SKIP_RPATH OFF)
set(CMAKE_SKIP_BUILD_RPATH OFF)
set(CMAKE_SKIP_INSTALL_RPATH OFF)

# ==============================================================================
# Component Toggles
# ==============================================================================
# Allows developers to selectively build only the required engine components.
option(SANDBOX_BUILD_ENGINE "Build the core sandbox engine library" ON)
option(SANDBOX_BUILD_LAUNCHER "Build the standalone engine launcher" ON)
option(SANDBOX_BUILD_TESTS "Build the automated testing suite" OFF)
option(SANDBOX_BUILD_EXAMPLES "Build the provided engine examples" OFF)

# ==============================================================================
# Developer Experience (DX)
# ==============================================================================
# Improves compilation times and enforces strict coding standards.
option(SANDBOX_ENABLE_PCH "Enable Precompiled Headers" ON)
option(SANDBOX_ENABLE_CCACHE "Enable ccache for faster recompilation" ON)
option(SANDBOX_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(SANDBOX_ENABLE_LTO "Enable Interprocedural Optimization (LTO)" OFF)

if(SANDBOX_ENABLE_CCACHE)
    find_program(CCACHE_PROGRAM ccache)
    if(CCACHE_PROGRAM)
        set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    endif()
endif()

if(SANDBOX_WARNINGS_AS_ERRORS)
    if(MSVC)
        add_compile_options(/WX)
    else()
        add_compile_options(-Werror)
    endif()
endif()

if(SANDBOX_ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT lto_supported OUTPUT lto_output)
    if(lto_supported)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        message(WARNING "LTO is not supported: ${lto_output}")
    endif()
endif()

# ==============================================================================
# Sanitizers (Memory Safety)
# ==============================================================================
# Enforce runtime safety checks during development.
set(SANDBOX_SANITIZER "None" CACHE STRING "Sanitizer to enable (None, Address, Thread, Undefined)")
set_property(CACHE SANDBOX_SANITIZER PROPERTY STRINGS "None" "Address" "Thread" "Undefined")

if(NOT SANDBOX_SANITIZER STREQUAL "None")
    if(MSVC)
        if(SANDBOX_SANITIZER STREQUAL "Address")
            add_compile_options(/fsanitize=address)
        else()
            message(WARNING "Sanitizer '${SANDBOX_SANITIZER}' is not natively supported by MSVC")
        endif()
    else()
        string(TOLOWER "${SANDBOX_SANITIZER}" sanitizer_lower)
        add_compile_options("-fsanitize=${sanitizer_lower}")
        add_link_options("-fsanitize=${sanitizer_lower}")
    endif()
endif()

# MARK: - Output Configuration
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ==============================================================================
# Engine Subdirectories
# ==============================================================================
if(SANDBOX_BUILD_ENGINE)
    add_subdirectory(sandbox)
endif()

# Only compile the standalone launcher, tests, and examples if we are
# compiling the Sandbox repository directly.
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    if(SANDBOX_BUILD_LAUNCHER)
        add_subdirectory(launcher)
    endif()
    
    if(SANDBOX_BUILD_TESTS)
        add_subdirectory(tests)
    endif()
    
    if(SANDBOX_BUILD_EXAMPLES AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples")
        add_subdirectory(examples)
    endif()
endif()

# ==============================================================================
# External Plugin Workspace Injector
# ==============================================================================
# Allows external developers to build their plugins against the engine 
# seamlessly without modifying the core engine's source code tree.
set(SANDBOX_EXTERNAL_MODULES_DIR "" CACHE PATH "Path to external user plugins directory")

if(NOT SANDBOX_EXTERNAL_MODULES_DIR STREQUAL "")
    message(STATUS "Integrating external user plugins from: ${SANDBOX_EXTERNAL_MODULES_DIR}")
    add_subdirectory("${SANDBOX_EXTERNAL_MODULES_DIR}" "${CMAKE_BINARY_DIR}/external_modules")
endif()