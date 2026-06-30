cmake_minimum_required(VERSION 3.20)
project(sandbox_core_plugin VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
set(SOURCES
    source/configuration/configuration.cpp
    source/filesystem/library.cpp
    source/filesystem/filesystem.cpp
    source/logs/logger.cpp
    source/runtime/library.cpp
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
    include/sandbox/sdk/configuration.hpp
    include/sandbox/sdk/filesystem.hpp
    include/sandbox/sdk/logs.hpp
    include/sandbox/sdk/runtime.hpp
)

# Create shared library
add_library(core_plugin MODULE ${SOURCES} ${HEADERS})
set_target_properties(core_plugin PROPERTIES
    PREFIX ""
)

# Includes
target_include_directories(core_plugin PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/source
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(core_plugin PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

include(FetchContent)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.15.3
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(
    miniz
    GIT_REPOSITORY https://github.com/richgel999/miniz.git
    GIT_TAG 3.0.2
    GIT_SHALLOW TRUE
    PATCH_COMMAND sed -i "s/cmake_minimum_required(VERSION 3.0)/cmake_minimum_required(VERSION 3.5)/g" CMakeLists.txt
)
FetchContent_MakeAvailable(miniz)

# Link against core engine and deps
target_link_libraries(core_plugin PRIVATE 
    sandbox::library
    spdlog::spdlog
    miniz
)

# Copy to bin directory
add_custom_command(TARGET core_plugin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    $<TARGET_FILE:core_plugin>
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<TARGET_FILE_NAME:core_plugin>
)
