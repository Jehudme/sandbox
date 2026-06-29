project(analytics LANGUAGES C CXX)

add_library(analytics MODULE
    source/library.cpp
)

target_include_directories(analytics
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(analytics PRIVATE sandbox::library)
