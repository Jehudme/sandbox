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
    versioning_test.cpp
    bootstrapper_lock_test.cpp
    architecture_test.cpp
    integration_test.cpp
)

target_link_libraries(sandbox_tests PRIVATE 
    sandbox::library 
    sandbox::sdk
    Catch2::Catch2
)

target_include_directories(sandbox_tests PRIVATE 
    ${CMAKE_SOURCE_DIR}/sandbox/source
)

# Set up test-specific output directories for isolation
set(TEST_RUNTIME_DIR ${CMAKE_BINARY_DIR}/tests_bin)
set_target_properties(sandbox_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_RUNTIME_DIR})

# Compile the mock plugin (libtest) for testing the bootstrapper and dynamic loader.
add_library(test_lib_mock MODULE ../libtest/source/library.cpp)
target_link_libraries(test_lib_mock PRIVATE sandbox::library sandbox::sdk)

set_target_properties(test_lib_mock PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${TEST_RUNTIME_DIR}/modules
    PREFIX ""
)

add_dependencies(sandbox_tests test_lib_mock)

add_custom_command(TARGET sandbox_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${TEST_RUNTIME_DIR}/app_mount
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/libtest/assets/manifest.json ${TEST_RUNTIME_DIR}/app_mount/manifest.json
)

include(CTest)
include(Catch)
catch_discover_tests(sandbox_tests)
