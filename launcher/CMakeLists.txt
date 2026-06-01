cmake_minimum_required(VERSION 3.20)
project(launcher LANGUAGES CXX)

add_executable(launcher source/main.cpp)
add_executable(sandbox::launcher ALIAS launcher)

target_link_libraries(launcher PRIVATE sandbox::library)

target_precompile_headers(launcher
        PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/include/launcher/pch.h>"
)

set_target_properties(launcher PROPERTIES
        OUTPUT_NAME "sandbox_engine"
        BUILD_RPATH "\$ORIGIN"
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH "\$ORIGIN"
)

add_dependencies(launcher test_lib)