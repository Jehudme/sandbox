project(configuration LANGUAGES C CXX)

add_library(configuration MODULE source/library.cpp)

target_include_directories(configuration
    PUBLIC include
)

target_link_libraries(configuration PRIVATE sandbox::library)
