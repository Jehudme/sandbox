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
    src/properties/test_properties_abi.cpp

    # Bootstrapper feature group
    src/bootstrapper/test_bootstrapper_staging.cpp
    src/bootstrapper/test_bootstrapper_activate.cpp
    src/bootstrapper/test_bootstrapper_boot.cpp
    src/bootstrapper/test_bootstrapper_abi_macros.cpp

    # Core systems feature group
    src/core/test_library_loader.cpp
)

# =============================================================================
# EXECUTABLE
# =============================================================================
add_executable(sandbox_unit_tests ${SANDBOX_UNIT_TEST_SOURCES})

target_link_libraries(sandbox_unit_tests
    PRIVATE
        sandbox::library
        Catch2::Catch2WithMain
)

target_include_directories(sandbox_unit_tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../../sandbox/source
)

# Ensure the dummy plugin shared library is built before the test executable
add_dependencies(sandbox_unit_tests dummy_plugin)

# Register tests with CTest
catch_discover_tests(sandbox_unit_tests)
