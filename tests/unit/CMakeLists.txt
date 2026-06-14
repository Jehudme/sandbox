cmake_minimum_required(VERSION 3.20)

# Add all unit test source files directly to the sandbox_tests executable
# This allows testing everything in one shot in CLion without merging folders.
target_sources(sandbox_tests PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/core/properties_test.cpp
)
