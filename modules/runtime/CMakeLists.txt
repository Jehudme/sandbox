project(runtime LANGUAGES C CXX)

add_library(runtime MODULE
    source/library.cpp
        source/runtime.cpp
        source/runtime.h
)
set_target_properties(runtime PROPERTIES PREFIX "")

target_include_directories(runtime
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../logs/include
)

target_link_libraries(runtime PRIVATE sandbox::library)
