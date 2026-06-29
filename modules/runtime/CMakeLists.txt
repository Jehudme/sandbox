project(runtime LANGUAGES C CXX)

add_library(runtime MODULE
    source/library.cpp
)

target_include_directories(runtime
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(runtime PRIVATE sandbox::library)
