project(events LANGUAGES C CXX)

add_library(events MODULE
    source/library.cpp
        source/events.cpp
        source/events.h
        include/sandbox/abi/events.h
        include/sandbox/sdk/events.hpp
)

set_target_properties(events PROPERTIES PREFIX "")

target_include_directories(events
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../logs/include
)

target_link_libraries(events PRIVATE sandbox::library)
