project(logs LANGUAGES C CXX)

# 1. Include the CMake FetchContent module
include(FetchContent)

# 2. Declare the spdlog dependency
FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG        v1.15.3
)

# 3. Make the spdlog targets available to your project
FetchContent_MakeAvailable(spdlog)

add_library(logs MODULE
        source/logger.cpp
        source/logger.h)

target_include_directories(logs
        PUBLIC include
)

# 4. Link spdlog::spdlog privately to your module
target_link_libraries(logs
        PRIVATE
        sandbox::library
        spdlog::spdlog
)
