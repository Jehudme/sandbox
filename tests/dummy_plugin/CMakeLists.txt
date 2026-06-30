cmake_minimum_required(VERSION 3.20)
project(dummy_plugin LANGUAGES CXX)

# Build a minimal shared library used by library_loader tests.
# Output ends up in the same bin/ include as the test executables so tests
# can reference it via a relative path from the executable location.
add_library(dummy_plugin SHARED source/library.cpp)

target_include_directories(dummy_plugin
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_SOURCE_DIR}/sandbox/include
)

target_compile_features(dummy_plugin PUBLIC cxx_std_23)

set_target_properties(dummy_plugin PROPERTIES
    PREFIX ""           # no "lib" prefix on Linux → "dummy_plugin.so"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

target_link_libraries(dummy_plugin PRIVATE flecs_headers)

target_precompile_headers(dummy_plugin PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_SOURCE_DIR}/sandbox/source/pch.h>")
