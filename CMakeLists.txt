cmake_minimum_required(VERSION 3.20)

project(sandbox_workspace VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_SKIP_RPATH OFF)
set(CMAKE_SKIP_BUILD_RPATH OFF)
set(CMAKE_SKIP_INSTALL_RPATH OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wno-missing-field-initializers)
endif()

option(SANDBOX_ENABLE_LTO "Enable Interprocedural Optimization (LTO)" OFF)
if(SANDBOX_ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported OUTPUT error)
    if(ipo_supported)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
    else()
        message(WARNING "IPO / LTO is not supported: ${error}")
    endif()
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

option(SANDBOX_BUILD_LAUNCHER "Build the default engine launcher" ON)
option(SANDBOX_BUILD_TESTING "Build the engine unit and suite tests" ON)

add_subdirectory(sandbox)
add_subdirectory(modules)

if(SANDBOX_BUILD_LAUNCHER)
    add_subdirectory(launcher)
endif()

if(SANDBOX_BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

# =============================================================================
# INSTALLATION & EXPORTS
# =============================================================================
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/sandboxConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/sandboxConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/sandbox"
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/sandboxConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/sandboxConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/sandboxConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/sandbox"
)
