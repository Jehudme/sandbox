project(analytics LANGUAGES C CXX)

add_library(analytics MODULE source/library.cpp)

target_include_directories(analytics
    PUBLIC include
)

target_link_libraries(analytics PRIVATE sandbox::library)
