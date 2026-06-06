cmake_minimum_required(VERSION 3.20)
project(sandbox_tests CXX)

include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.5.3
)
FetchContent_MakeAvailable(Catch2)

add_executable(sandbox_tests 
    main.cpp
    unit/core/properties_test.cpp
    unit/core/bootstrapper_test.cpp
    unit/event_bus/event_bus_test.cpp
    unit/subsystems/filesystem_test.cpp
    unit/subsystems/logger_test.cpp
    unit/subsystems/runner_test.cpp
    integration/plugin_loader_test.cpp
)

# We link against Catch2::Catch2 (NOT Catch2WithMain) since we are providing our own main.cpp
target_link_libraries(sandbox_tests PRIVATE 
    sandbox::library 
    Catch2::Catch2
)

target_include_directories(sandbox_tests PRIVATE 
    ${CMAKE_SOURCE_DIR}/sandbox/source
)

# Set up test-specific output directories for isolation
set(TEST_RUNTIME_DIR ${CMAKE_BINARY_DIR}/tests_bin)
set_target_properties(sandbox_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_RUNTIME_DIR})

# Compile the mock plugin (libtest) for testing the bootstrapper and dynamic loader.
# Output it to our test-specific mount://bin/modules directory
add_library(test_lib_mock MODULE ../libtest/source/library.cpp)
target_link_libraries(test_lib_mock PRIVATE sandbox::library)

set_target_properties(test_lib_mock PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${TEST_RUNTIME_DIR}/modules
    PREFIX ""
)

# Ensure sandbox_tests builds the mock plugin
add_dependencies(sandbox_tests test_lib_mock)

# Copy the required manifest into a test-specific app mount directory
add_custom_command(TARGET sandbox_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${TEST_RUNTIME_DIR}/app_mount
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/libtest/assets/manifest.json ${TEST_RUNTIME_DIR}/app_mount/manifest.json
)

# Expose CTest integration
include(CTest)
include(Catch)
catch_discover_tests(sandbox_tests)
