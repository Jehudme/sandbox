project(runtime LANGUAGES C CXX)

add_library(runtime MODULE source/library.cpp)

target_include_directories(runtime
    PUBLIC include
)

target_link_libraries(runtime PRIVATE sandbox::library)
