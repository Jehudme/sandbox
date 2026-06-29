project(filesystem LANGUAGES C CXX)

add_library(filesystem MODULE
    source/library.cpp
)

target_include_directories(filesystem
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(filesystem PRIVATE sandbox::library)
