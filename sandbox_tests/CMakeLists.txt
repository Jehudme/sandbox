cmake_minimum_required(VERSION 3.20)

project(sandbox_tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# =============================================================================
# DEPENDENCIES: Catch2
# =============================================================================
include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.8.1
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(Catch2)

# Make Catch2 CMake helpers available (catch_discover_tests, etc.)
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)

# =============================================================================
# SANDBOX LIBRARY (from the sibling sandbox/ sub-project)
# =============================================================================
add_subdirectory(
    ${CMAKE_CURRENT_SOURCE_DIR}/../sandbox
    ${CMAKE_CURRENT_BINARY_DIR}/sandbox_library
)

# =============================================================================
# SUB-PROJECTS
# =============================================================================
add_subdirectory(unit)
add_subdirectory(suite)
