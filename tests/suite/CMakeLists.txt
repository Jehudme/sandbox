cmake_minimum_required(VERSION 3.20)
project(sandbox_suite CXX)

add_executable(sandbox_tests 
    main.cpp
    versioning_test.cpp
    bootstrapper_lock_test.cpp
    architecture_test.cpp
    integration_test.cpp
    plugin_loader_test.cpp
    graph_safety_test.cpp
    launcher_test.cpp
)

target_link_libraries(sandbox_tests PRIVATE 
    sandbox::library 
    Catch2::Catch2
)

target_include_directories(sandbox_tests PRIVATE 
    ${CMAKE_SOURCE_DIR}/sandbox/source
)

add_dependencies(sandbox_tests test_lib_mock launcher)

# Prevent Root Pollution: Copy mock plugin to a local test_mount directory
set(TEST_MOUNT_DIR "${CMAKE_CURRENT_BINARY_DIR}/test_mount")

add_custom_command(TARGET sandbox_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${TEST_MOUNT_DIR}/modules
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:test_lib_mock> ${TEST_MOUNT_DIR}/modules/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/libtest/assets/manifest.json ${TEST_MOUNT_DIR}/manifest.json
    COMMAND ${CMAKE_COMMAND} -E chdir ${TEST_MOUNT_DIR} ${CMAKE_COMMAND} -E tar "cf" "test-app.zip" "--format=zip" "manifest.json" "modules"
)

target_compile_definitions(sandbox_tests PRIVATE 
    TEST_MOUNT_DIR="${TEST_MOUNT_DIR}"
    LAUNCHER_EXEC_PATH="$<TARGET_FILE:launcher>"
)

include(CTest)
include(Catch)
catch_discover_tests(sandbox_tests)
