project(events LANGUAGES C CXX)

add_library(events MODULE
    source/library.cpp
)

target_include_directories(events
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(events PRIVATE sandbox::library)
