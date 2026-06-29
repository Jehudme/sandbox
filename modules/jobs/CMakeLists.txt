project(jobs LANGUAGES C CXX)

add_library(jobs MODULE
    source/library.cpp
)

target_include_directories(jobs
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(jobs PRIVATE sandbox::library)
