cmake_minimum_required(VERSION 3.20)
project(sandbox_unit_tests CXX)

add_executable(sandbox_unit_tests
    main.cpp
    core/properties_c_api_test.cpp
)

target_link_libraries(sandbox_unit_tests PRIVATE 
    sandbox::library 
    Catch2::Catch2WithMain
)

target_include_directories(sandbox_unit_tests PRIVATE 
    ${CMAKE_SOURCE_DIR}/sandbox/source
)

include(CTest)
include(Catch)
catch_discover_tests(sandbox_unit_tests)
