cmake_minimum_required(VERSION 3.20)
project(sandbox_tests CXX)

include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.5.3
)
FetchContent_MakeAvailable(Catch2)

add_subdirectory(suite)
add_subdirectory(unit)
