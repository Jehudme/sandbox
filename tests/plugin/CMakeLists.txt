cmake_minimum_required(VERSION 3.20)
project(mock_plugin LANGUAGES CXX)

add_library(mock_plugin MODULE source/library.cpp)
target_link_libraries(mock_plugin PRIVATE sandbox::library)


set_target_properties(mock_plugin PROPERTIES PREFIX "")