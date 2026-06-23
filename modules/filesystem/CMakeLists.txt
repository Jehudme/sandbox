project(filesystem LANGUAGES C CXX)

add_library(filesystem MODULE source/library.cpp)

target_include_directories(filesystem
    PUBLIC include
)

target_link_libraries(filesystem PRIVATE sandbox::library)
