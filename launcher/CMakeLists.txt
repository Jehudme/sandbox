cmake_minimum_required(VERSION 3.20)
project(launcher LANGUAGES CXX)

# 1. Include CLI11 locally since it is now launcher-exclusive
include(FetchContent)
FetchContent_Declare(
        cli11
        GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
        GIT_TAG        v2.4.2
        GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(cli11)

# 2. Define the launcher executable
add_executable(launcher source/main.cpp)
add_executable(sandbox::launcher ALIAS launcher)

# 3. Link the core engine AND the launcher-specific CLI11 library
target_link_libraries(launcher PRIVATE
        sandbox::library
        CLI11::CLI11
)

# 4. Apply the Launcher Precompiled Header
target_precompile_headers(launcher
        PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/include/launcher/pch.h>"
)

# 5. Strict Linux Linker RPATH Configuration
# Ensures the executable always looks in its local directory ($ORIGIN) for libsandbox.so
set_target_properties(launcher PROPERTIES
        OUTPUT_NAME "sandbox"
        BUILD_RPATH "\$ORIGIN"
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH "\$ORIGIN"
)

# 6. Build Order Dependency
# Ensures plugins are compiled before the final launcher link
add_dependencies(launcher test_lib)