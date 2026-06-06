cmake_minimum_required(VERSION 3.20)

# Rename the root project so it doesn't collide with the inner library project
project(sandbox_root LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_SKIP_RPATH OFF)
set(CMAKE_SKIP_BUILD_RPATH OFF)
set(CMAKE_SKIP_INSTALL_RPATH OFF)

# MARK: - Output Configuration
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ALWAYS build the core engine library (This is what Harmony needs)
add_subdirectory(sandbox)

# MARK: - Subprojects
# Only compile the standalone launcher and plugins if we are
# compiling the Sandbox repository directly.
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    add_subdirectory(launcher)
    add_subdirectory(libtest)
    add_subdirectory(tests)
endif()