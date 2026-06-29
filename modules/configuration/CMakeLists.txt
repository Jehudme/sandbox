project(configuration LANGUAGES C CXX)

add_library(configuration MODULE
        source/configuration.cpp
        source/configuration.h)

target_include_directories(configuration
    PUBLIC include
)

target_link_libraries(configuration PRIVATE sandbox::library)
