project(configuration LANGUAGES C CXX)

add_library(configuration MODULE
    source/configuration.cpp
)

target_include_directories(configuration
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(configuration PRIVATE sandbox::library)
