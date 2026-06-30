project(configuration LANGUAGES C CXX)

add_library(configuration MODULE
    source/library.cpp
    source/configuration.cpp
)

set_target_properties(configuration PROPERTIES PREFIX "")

target_include_directories(configuration
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(configuration PRIVATE sandbox::library)
