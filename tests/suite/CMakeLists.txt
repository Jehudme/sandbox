cmake_minimum_required(VERSION 3.20)

project(sandbox_suite_tests LANGUAGES C CXX)

set(SANDBOX_SUITE_TEST_SOURCES
    src/suite_full_boot_pipeline.cpp
    src/suite_complex_dependency_graph.cpp
    src/suite_properties_serialization_roundtrip.cpp
    src/suite_service_collision_resolution.cpp
    src/suite_library_loading.cpp
)

# =============================================================================
# EXECUTABLE
# =============================================================================
add_executable(sandbox_suite_tests ${SANDBOX_SUITE_TEST_SOURCES})
set_target_properties(sandbox_suite_tests PROPERTIES ENABLE_EXPORTS ON)

target_link_libraries(sandbox_suite_tests
    PRIVATE
        sandbox::library
        Catch2::Catch2WithMain
        -Wl,--whole-archive $<TARGET_FILE:flecs_static> -Wl,--no-whole-archive
)

target_include_directories(sandbox_suite_tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../../sandbox/source
)

# Ensure the shared libraries are built before the test executable
add_dependencies(sandbox_suite_tests dummy_plugin core_plugin)

catch_discover_tests(sandbox_suite_tests)

target_precompile_headers(sandbox_suite_tests PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_SOURCE_DIR}/sandbox/source/pch.h>")
