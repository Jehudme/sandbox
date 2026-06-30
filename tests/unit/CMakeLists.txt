cmake_minimum_required(VERSION 3.20)

project(sandbox_unit_tests LANGUAGES C CXX)

# =============================================================================
# SOURCE FILES
# =============================================================================
set(SANDBOX_UNIT_TEST_SOURCES
    # Properties feature group
    src/properties/test_properties_lifecycle.cpp
    src/properties/test_properties_load_dump.cpp
    src/properties/test_properties_getters_setters.cpp
    src/properties/test_properties_tree_manipulation.cpp
    src/properties/test_properties_sdk.cpp
    src/properties/test_properties_abi.cpp

    # Bootstrapper feature group
    src/bootstrapper/test_bootstrapper_staging.cpp
    src/bootstrapper/test_bootstrapper_activate.cpp
    src/bootstrapper/test_bootstrapper_boot.cpp
    src/bootstrapper/test_bootstrapper_abi_macros.cpp
    src/bootstrapper/test_bootstrapper_indexing.cpp
    src/bootstrapper/test_bootstrapper_activate_string.cpp
    src/bootstrapper/test_bootstrapper_sdk.cpp

    # Core systems feature group
    src/core/test_library_loader.cpp
    src/core/test_cli_parser.cpp
    src/core/test_engine_bootstrapper.cpp
    src/core/test_engine_sdk.cpp
    src/modules/test_configuration.cpp
    src/modules/test_logs.cpp
    src/modules/test_runtime.cpp
    src/modules/test_filesystem.cpp
    ../../launcher/source/cli/cli_parser.cpp
)

# =============================================================================
# EXECUTABLE
# =============================================================================
add_executable(sandbox_unit_tests ${SANDBOX_UNIT_TEST_SOURCES})
set_target_properties(sandbox_unit_tests PROPERTIES ENABLE_EXPORTS ON)

target_link_libraries(sandbox_unit_tests
    PRIVATE
        sandbox::library
        Catch2::Catch2WithMain
        CLI11::CLI11
        -Wl,--whole-archive $<TARGET_FILE:flecs_static> -Wl,--no-whole-archive
)

target_include_directories(sandbox_unit_tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../../sandbox/source
        ${CMAKE_CURRENT_SOURCE_DIR}/../../modules/core/include
)

# Ensure the shared libraries are built before the test executable
add_dependencies(sandbox_unit_tests dummy_plugin core_plugin)

# Register tests with CTest
catch_discover_tests(sandbox_unit_tests)

target_precompile_headers(sandbox_unit_tests PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_SOURCE_DIR}/sandbox/source/pch.h>")
