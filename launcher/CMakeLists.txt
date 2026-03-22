cmake_minimum_required(VERSION 3.20)
project(launcher LANGUAGES CXX)

add_executable(launcher
        source/main.cpp
)

add_executable(sandbox::launcher ALIAS launcher)

target_link_libraries(launcher PRIVATE sandbox::library)