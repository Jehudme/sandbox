cmake_minimum_required(VERSION 3.20)
project(sandbox_suite CXX)

add_executable(sandbox_tests 
    main.cpp
)

target_link_libraries(sandbox_tests PRIVATE 
    sandbox::library 
    Catch2::Catch2
)

target_include_directories(sandbox_tests PRIVATE 
    ${CMAKE_SOURCE_DIR}/sandbox/source
)

add_dependencies(sandbox_tests mock_plugin sandbox_launcher)

include(CTest)
include(Catch)
catch_discover_tests(sandbox_tests)
