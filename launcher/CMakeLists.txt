cmake_minimum_required(VERSION 3.20)
project(sandbox_launcher LANGUAGES CXX)

# Include CLI11 locally since it is now launcher-exclusive
include(FetchContent)
FetchContent_Declare(
        cli11
        GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
        GIT_TAG        v2.4.2
        GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(cli11)

# Define the launcher executable
add_executable(sandbox_launcher source/main.cpp)
add_executable(sandbox::launcher ALIAS sandbox_launcher)

# Link the core engine AND the launcher-specific CLI11 library
target_link_libraries(sandbox_launcher PRIVATE
        sandbox::library
        CLI11::CLI11
)

# Apply the Launcher Precompiled Header
target_precompile_headers(sandbox_launcher
        PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/include/launcher/pch.h>"
)

set_target_properties(sandbox_launcher PROPERTIES
        OUTPUT_NAME "sandbox"
        BUILD_RPATH "\$ORIGIN"
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH "\$ORIGIN"
)

