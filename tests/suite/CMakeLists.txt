cmake_minimum_required(VERSION 3.20)

project(sandbox_suite_tests LANGUAGES C CXX)

set(SANDBOX_SUITE_TEST_SOURCES
    src/suite_full_boot_pipeline.cpp
    src/suite_complex_dependency_graph.cpp
    src/suite_properties_serialization_roundtrip.cpp
    src/suite_service_collision_resolution.cpp
)

# =============================================================================
# EXECUTABLE
# =============================================================================
add_executable(sandbox_suite_tests ${SANDBOX_SUITE_TEST_SOURCES})

target_link_libraries(sandbox_suite_tests
    PRIVATE
        sandbox::library
        Catch2::Catch2WithMain
)

target_include_directories(sandbox_suite_tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../../sandbox/source
)

catch_discover_tests(sandbox_suite_tests)
