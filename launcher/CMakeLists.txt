cmake_minimum_required(VERSION 3.20)
project(launcher LANGUAGES CXX)

add_executable(launcher
        source/main.cpp
)

add_executable(sandbox::launcher ALIAS launcher)

target_link_libraries(launcher PRIVATE sandbox::library)

target_precompile_headers(launcher
        PRIVATE
        "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/include/launcher/pch.h>"
)

set_target_properties(launcher PROPERTIES
        OUTPUT_NAME "sandbox_engine"
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
        ENABLE_EXPORTS TRUE
)