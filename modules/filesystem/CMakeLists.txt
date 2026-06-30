project(filesystem LANGUAGES C CXX)

add_library(filesystem MODULE
    source/library.cpp
        source/filesystem.cpp
        source/filesystem.h
)

set_target_properties(filesystem PROPERTIES PREFIX "")

target_include_directories(filesystem
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../configuration/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../logs/include
)

include(FetchContent)
FetchContent_Declare(
    miniz
    GIT_REPOSITORY https://github.com/richgel999/miniz.git
    GIT_TAG 3.0.2
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(miniz)

target_link_libraries(filesystem PRIVATE sandbox::library miniz)
