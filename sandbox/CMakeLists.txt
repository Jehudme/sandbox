cmake_minimum_required(VERSION 3.20)
project(sandbox_library LANGUAGES CXX)

set(CMAKE_POLICY_VERSION_MINIMUM "3.5")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(FetchContent)

FetchContent_Declare(spdlog GIT_REPOSITORY https://github.com/gabime/spdlog.git GIT_TAG v1.15.0 GIT_SHALLOW TRUE)
FetchContent_Declare(glaze GIT_REPOSITORY https://github.com/stephenberry/glaze.git GIT_TAG v4.3.0 GIT_SHALLOW TRUE)
FetchContent_Declare(flecs GIT_REPOSITORY https://github.com/SanderMertens/flecs.git GIT_TAG v4.1.4 GIT_SHALLOW TRUE)
FetchContent_Declare(physfs GIT_REPOSITORY https://github.com/icculus/physfs.git GIT_TAG release-3.2.0 GIT_SHALLOW TRUE)
FetchContent_Declare(flatbuffers GIT_REPOSITORY https://github.com/google/flatbuffers.git GIT_TAG v23.5.26 GIT_SHALLOW TRUE)

set(FLECS_SHARED ON)
set(FLECS_STATIC OFF)
set(SPDLOG_BUILD_SHARED ON)
set(PHYSFS_BUILD_SHARED OFF)
set(PHYSFS_BUILD_STATIC ON)
set(FLATBUFFERS_BUILD_TESTS OFF)
set(FLATBUFFERS_BUILD_FLATC ON)
set(glaze_ENABLE_AVX2 OFF)
FetchContent_MakeAvailable(spdlog glaze flecs physfs flatbuffers)

# MARK: - FlatBuffers Code Generation
set(FBS_LOGGER ${CMAKE_CURRENT_SOURCE_DIR}/schemas/logger.fbs)
set(FBS_RUNNER ${CMAKE_CURRENT_SOURCE_DIR}/schemas/runner.fbs)
set(FBS_FILESYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/schemas/filesystem.fbs)
set(FBS_COMMON ${CMAKE_CURRENT_SOURCE_DIR}/schemas/common.fbs)

set(FBS_LOGGER_OUT ${CMAKE_CURRENT_SOURCE_DIR}/include/sandbox/modules/logger/logger_schema.h)
set(FBS_RUNNER_OUT ${CMAKE_CURRENT_SOURCE_DIR}/include/sandbox/modules/runner/runner_schema.h)
set(FBS_FILESYSTEM_OUT ${CMAKE_CURRENT_SOURCE_DIR}/include/sandbox/modules/filesystem/filesystem_schema.h)
set(FBS_COMMON_OUT ${CMAKE_CURRENT_SOURCE_DIR}/include/sandbox/core/common_schema.h)

add_custom_command(
        OUTPUT ${FBS_LOGGER_OUT}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_SOURCE_DIR}/include/sandbox/modules/logger
        COMMAND flatc --cpp --gen-object-api -o ${CMAKE_CURRENT_SOURCE_DIR}/include/sandbox/modules/logger ${FBS_LOGGER}
        COMMAND ${CMAKE_COMMAND} -E rename ${CMAKE_CURRENT_SOURCE_DIR}/include/sandbox/modules/logger/logger_generated.h ${FBS_LOGGER_OUT}
        DEPENDS ${FBS_LOGGER} flatc
)
add_custom_command(
        OUTPUT ${FBS_RUNNER_OUT}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_SOURCE_DIR}/include/sandbox/modules/runner
        COMMAND flatc --cpp --gen-object-api -o ${CMAKE_CURRENT_SOURCE_DIR}/include/sandbox/modules/runner ${FBS_RUNNER}
        COMMAND ${CMAKE_COMMAND} -E rename ${CMAKE_CURRENT_SOURCE_DIR}/include/sandbox/modules/runner/runner_generated.h ${FBS_RUNNER_OUT}
        DEPENDS ${FBS_RUNNER} flatc
)
add_custom_command(
        OUTPUT ${FBS_FILESYSTEM_OUT}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_SOURCE_DIR}/include/sandbox/modules/filesystem
        COMMAND flatc --cpp --gen-object-api -o ${CMAKE_CURRENT_SOURCE_DIR}/include/sandbox/modules/filesystem ${FBS_FILESYSTEM}
        COMMAND ${CMAKE_COMMAND} -E rename ${CMAKE_CURRENT_SOURCE_DIR}/include/sandbox/modules/filesystem/filesystem_generated.h ${FBS_FILESYSTEM_OUT}
        DEPENDS ${FBS_FILESYSTEM} flatc
)
add_custom_command(
        OUTPUT ${FBS_COMMON_OUT}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_SOURCE_DIR}/include/sandbox/core
        COMMAND flatc --cpp --gen-object-api -o ${CMAKE_CURRENT_SOURCE_DIR}/include/sandbox/core ${FBS_COMMON}
        COMMAND ${CMAKE_COMMAND} -E rename ${CMAKE_CURRENT_SOURCE_DIR}/include/sandbox/core/common_generated.h ${FBS_COMMON_OUT}
        DEPENDS ${FBS_COMMON} flatc
)

# -----------------------------------------------------------------------------
# THE UNIFIED HOST ENGINE TARGET
# -----------------------------------------------------------------------------
add_library(sandbox_library SHARED
        ${FBS_LOGGER_OUT} ${FBS_RUNNER_OUT} ${FBS_FILESYSTEM_OUT} ${FBS_COMMON_OUT}
        source/core/bootstrapper.cpp
        source/core/properties.cpp
        source/core/engine.cpp
        source/core/engine_library.cpp
        source/core/plugin.cpp
        source/utilities/filesystem.cpp
        source/utilities/loader.cpp
        source/modules/logger/logger.cpp
        source/modules/runner/runner.cpp
        source/modules/filesystem/filesystem.cpp
)
add_library(sandbox::library ALIAS sandbox_library)

target_compile_definitions(sandbox_library PRIVATE SANDBOX_BUILD_SHARED)
target_compile_features(sandbox_library PUBLIC cxx_std_23)

target_include_directories(sandbox_library
        PUBLIC  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                $<INSTALL_INTERFACE:include>
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/source
)

target_link_libraries(sandbox_library PUBLIC
        flecs
        spdlog::spdlog
        glaze::glaze
        physfs-static
        flatbuffers
)

if(UNIX AND NOT APPLE)
    target_link_libraries(sandbox_library PUBLIC ${CMAKE_DL_LIBS})
endif()

target_precompile_headers(sandbox_library PRIVATE include/sandbox/pch.h)

set_target_properties(sandbox_library PROPERTIES
        BUILD_RPATH "\$ORIGIN"
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH "\$ORIGIN"
)