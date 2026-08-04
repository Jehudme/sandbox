cmake_minimum_required(VERSION 3.20)
project(sandbox_library LANGUAGES C CXX)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
option(BUILD_SHARED_LIBS "Build shared libraries" ON)

# =============================================================================
# 1. DEPENDENCIES
# =============================================================================
include(FetchContent)

FetchContent_Declare(
        glaze
        GIT_REPOSITORY https://github.com/stephenberry/glaze.git
        GIT_TAG v7.8.1
        GIT_SHALLOW TRUE
)

FetchContent_Declare(
        flecs
        GIT_REPOSITORY https://github.com/SanderMertens/flecs.git
        GIT_TAG v4.1.4
        GIT_SHALLOW TRUE
)

FetchContent_Declare(
        dylib
        GIT_REPOSITORY https://github.com/martin-olivier/dylib.git
        GIT_TAG v3.0.1
        GIT_SHALLOW TRUE
)

# Populate and make targets available
set(SANDBOX_ORIGINAL_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})
set(BUILD_SHARED_LIBS OFF)
FetchContent_MakeAvailable(glaze flecs dylib)
set(BUILD_SHARED_LIBS ${SANDBOX_ORIGINAL_BUILD_SHARED_LIBS})

set(FLECS_SHARED OFF CACHE INTERNAL "Build Flecs as a shared library")
set(FLECS_STATIC ON CACHE INTERNAL "Disable Flecs static library")
set(glaze_ENABLE_AVX2 OFF CACHE INTERNAL "Disable AVX2 for Glaze")

add_library(flecs_headers INTERFACE)
target_include_directories(flecs_headers INTERFACE 
    $<BUILD_INTERFACE:${flecs_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_definitions(flecs_headers INTERFACE flecs_STATIC)

if(TARGET flecs_static)
    set_target_properties(flecs_static PROPERTIES C_VISIBILITY_PRESET default CXX_VISIBILITY_PRESET default)
endif()

# =============================================================================
# 2. CORE LIBRARY TARGET
# =============================================================================
add_library(sandbox_library

        source/core/properties.h
        source/core/properties.cpp
        source/abi/properties.cpp
        include/sandbox/abi/properties.h
        source/core/bootstrapper.cpp
        source/core/bootstrapper.h
        include/sandbox/abi/bootstrapper.h
        source/abi/bootstrapper.cpp
        include/sandbox/abi/platform.h
        source/core/engine.cpp
        source/core/engine.h
        source/abi/engine.cpp
        include/sandbox/abi/engine.h
        source/core/library_loader.cpp
        source/core/library_loader.h
        include/sandbox/abi/handle.h
)

set_target_properties(sandbox_library PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
set_target_properties(sandbox_library PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_library(sandbox::library ALIAS sandbox_library)

add_library(sandbox_headers INTERFACE)
target_include_directories(sandbox_headers INTERFACE
    $<TARGET_PROPERTY:sandbox::plugins,INTERFACE_INCLUDE_DIRECTORIES>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(sandbox_headers INTERFACE flecs_headers)
add_library(sandbox::headers ALIAS sandbox_headers)

target_compile_definitions(sandbox_library PRIVATE SANDBOX_BUILD_SHARED)
target_compile_features(sandbox_library PUBLIC cxx_std_23)

target_include_directories(sandbox_library
    PUBLIC
        $<TARGET_PROPERTY:sandbox::plugins,INTERFACE_INCLUDE_DIRECTORIES>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/source
)

target_link_libraries(sandbox_library 
    PUBLIC
        flecs_headers
    PRIVATE
        glaze::glaze
        dylib
        flecs_static
)

# Apply the Launcher Precompiled Header
target_precompile_headers(sandbox_library
        PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/source/pch.h>"
)

# =============================================================================
# 3. INSTALLATION RULES
# =============================================================================
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(GNUInstallDirs)

    install(TARGETS sandbox_library flecs_headers
        EXPORT sandboxTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

    install(DIRECTORY include/sandbox
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(EXPORT sandboxTargets
        FILE sandboxTargets.cmake
        NAMESPACE sandbox::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sandbox
    )
endif()