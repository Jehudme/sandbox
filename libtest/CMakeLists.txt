cmake_minimum_required(VERSION 3.20)
project(test_lib LANGUAGES CXX)

# MODULE explicitly tells CMake this is a runtime plugin, not a standard library
add_library(test_lib MODULE source/library.cpp)

# Link against the shared engine.
target_link_libraries(test_lib PRIVATE sandbox::library)

# Output the plugin specifically to the "modules" folder inside the bin/ directory
# so your executable's VFS initialization sequence can find it instantly.
set_target_properties(test_lib PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/modules
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH "\$ORIGIN/.."
)