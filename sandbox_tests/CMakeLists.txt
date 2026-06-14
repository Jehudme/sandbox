cmake_minimum_required(VERSION 3.20)

project(sandbox_tests LANGUAGES CXX)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.8.1
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(Catch2)
include(Catch)


add_subdirectory(unit)
add_subdirectory(suite)
