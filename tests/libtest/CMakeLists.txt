cmake_minimum_required(VERSION 3.20)
project(test_lib_mock LANGUAGES CXX)

add_library(test_lib_mock MODULE source/library.cpp)
target_link_libraries(test_lib_mock PRIVATE sandbox::library)

set_target_properties(test_lib_mock PROPERTIES PREFIX "")