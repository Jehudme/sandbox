project(logs LANGUAGES C CXX)

add_library(logs MODULE
    source/logger.cpp
)

target_include_directories(logs
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
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
