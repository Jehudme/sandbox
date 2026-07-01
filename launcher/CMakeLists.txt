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

set(BUILD_SHARED_LIBS ${SANDBOX_ORIGINAL_BUILD_SHARED_LIBS})

add_executable(sandbox_launcher source/main.cpp
        source/cli/cli_parser.cpp
        source/cli/cli_parser.h
        source/application/application.cpp
        source/application/application.h)
add_executable(sandbox::launcher ALIAS sandbox_launcher)

set_target_properties(sandbox_launcher PROPERTIES ENABLE_EXPORTS ON)

target_link_libraries(sandbox_launcher PRIVATE
        sandbox::library
        CLI11::CLI11
        -Wl,--whole-archive $<TARGET_FILE:flecs_static> -Wl,--no-whole-archive
)

target_precompile_headers(sandbox_launcher PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_SOURCE_DIR}/sandbox/source/pch.h>")

# =============================================================================
# INSTALLATION
# =============================================================================
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(GNUInstallDirs)
    install(TARGETS sandbox_launcher
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()
