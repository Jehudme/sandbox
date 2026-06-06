cmake_minimum_required(VERSION 3.20)
project(sandbox_sdk LANGUAGES CXX)

add_library(sdk INTERFACE)
add_library(sandbox::sdk ALIAS sdk)

target_compile_features(sdk INTERFACE cxx_std_23)
target_include_directories(sdk INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# If SDK needs flecs/spdlog etc, they can be linked here, but since it's just interfaces,
# it might not strictly need them, or it might for flecs::world if passed around.
# event_bus uses them, so we will link flecs and spdlog.
target_link_libraries(sdk INTERFACE flecs spdlog::spdlog)
