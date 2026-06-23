project(jobs LANGUAGES C CXX)

add_library(jobs MODULE source/library.cpp)

target_include_directories(jobs
    PUBLIC include
)

target_link_libraries(jobs PRIVATE sandbox::library)
