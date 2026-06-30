cmake_minimum_required(VERSION 3.20)
project(sandbox_launcher LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
        cli11
        GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
        GIT_TAG        v2.4.2
        GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(cli11)

add_executable(sandbox_launcher source/main.cpp
        source/cli/cli_parser.cpp
        source/cli/cli_parser.h)
add_executable(sandbox::launcher ALIAS sandbox_launcher)

target_link_libraries(sandbox_launcher PRIVATE
        sandbox::library
        CLI11::CLI11
)

target_precompile_headers(sandbox_launcher PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_SOURCE_DIR}/sandbox/source/pch.h>")

