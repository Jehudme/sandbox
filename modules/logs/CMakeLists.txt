project(logs LANGUAGES C CXX)

add_library(logs MODULE
    source/logger.cpp
    source/library.cpp
)

set_target_properties(logs PROPERTIES PREFIX "")

target_include_directories(logs
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../configuration/include
)

include(FetchContent)
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.15.3
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(spdlog)

target_link_libraries(logs PRIVATE sandbox::library spdlog::spdlog)
