cmake_minimum_required(VERSION 3.20)
project(sandbox_launcher LANGUAGES CXX)

include(FetchContent)
set(SANDBOX_ORIGINAL_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})
set(BUILD_SHARED_LIBS OFF)

find_package(cli11 QUIET)
if(NOT cli11_FOUND)
    FetchContent_Declare(
            cli11
            GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
            GIT_TAG        v2.4.2
            GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(cli11)
endif()

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

set(BUILD_SHARED_LIBS ${SANDBOX_ORIGINAL_BUILD_SHARED_LIBS})

add_executable(sandbox_launcher source/main.cpp
        source/cli/cli_parser.cpp
        source/cli/cli_parser.h)
add_executable(sandbox::launcher ALIAS sandbox_launcher)

set_target_properties(sandbox_launcher PROPERTIES ENABLE_EXPORTS ON)
add_dependencies(sandbox_launcher sandbox_plugin_static flecs_static)

target_link_libraries(sandbox_launcher PRIVATE
        sandbox::library
        CLI11::CLI11
        -Wl,--whole-archive $<TARGET_FILE:sandbox_plugin_static> -Wl,--no-whole-archive
        spdlog::spdlog
        miniz
        -Wl,--whole-archive $<TARGET_FILE:flecs_static> -Wl,--no-whole-archive
)

target_precompile_headers(sandbox_launcher PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/../sandbox/source/pch.h>")

# =============================================================================
# INSTALLATION
# =============================================================================
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(GNUInstallDirs)
    install(TARGETS sandbox_launcher
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()
