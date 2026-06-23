project(logs LANGUAGES C CXX)

add_library(logs MODULE source/library.cpp)

target_include_directories(logs
    PUBLIC include
)

target_link_libraries(logs PRIVATE sandbox::library)
