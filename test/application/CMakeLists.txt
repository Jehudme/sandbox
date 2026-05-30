# -----------------------------------------------------------------------------
# Mock Plugin Library (For Testing the Loader)
# -----------------------------------------------------------------------------

# We use 'MODULE' here to explicitly tell CMake this is a runtime-loaded plugin
add_library(mock_test_plugin MODULE
        library.cpp
)

# Link it against Flecs and your core Sandbox engine headers
target_link_libraries(mock_test_plugin
        PRIVATE
        sandbox::library
        flecs
)

# CRITICAL STEP: Force CMake to output the compiled .dll/.so directly into
# the "libraries/" folder so your engine's auto-scanner finds it immediately.
set_target_properties(mock_test_plugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/libraries
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/libraries
)