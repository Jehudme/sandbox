cmake_minimum_required(VERSION 3.20)
project(launcher LANGUAGES CXX)

add_executable(launcher
        source/main.cpp
)

add_executable(sandbox::launcher ALIAS launcher)

# REMOVED mock_test_plugin FROM HERE:
target_link_libraries(launcher PRIVATE sandbox::library)

# ADDED THIS: Tell CMake to compile the plugin before the launcher runs, but don't link them!
add_dependencies(launcher mock_test_plugin)

target_precompile_headers(launcher
        PRIVATE
        "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/include/launcher/pch.h>"
)

set_target_properties(launcher PROPERTIES
        OUTPUT_NAME "sandbox_engine"
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
        ENABLE_EXPORTS TRUE
)