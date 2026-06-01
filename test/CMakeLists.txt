# We use MODULE to tell CMake this is a runtime-loaded plugin
add_library(test_lib MODULE
        library.cpp
)

# Link against the engine core
target_link_libraries(test_lib PRIVATE sandbox::library)

# Force the compiled .so to be generated exactly where the VFS mounts the launcher
set_target_properties(test_lib PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/libraries
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/libraries
)