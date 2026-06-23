project(events LANGUAGES C CXX)

add_library(events MODULE source/library.cpp)

target_include_directories(events
    PUBLIC include
)

target_link_libraries(events PRIVATE sandbox::library)
