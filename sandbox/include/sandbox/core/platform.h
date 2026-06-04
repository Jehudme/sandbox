#pragma once

// MARK: - Explicit Platform Detection

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
    #define SANDBOX_WINDOWS_PLATFORM

#elif defined(__APPLE__) && defined(__MACH__)
    #define SANDBOX_APPLE_PLATFORM

#elif defined(__linux__)
    #define SANDBOX_LINUX_PLATFORM

#else
    #error "[Sandbox Engine] Compilation failed: Unsupported or unknown platform detected!"
#endif


// MARK: - Platform-Specific Export & File Configuration

#if defined(SANDBOX_WINDOWS_PLATFORM)
    // Used by Engine Classes (Exports when building the engine, Imports when consumed by launcher/plugins)
    #if defined(SANDBOX_BUILD_SHARED)
        #define SANDBOX_API __declspec(dllexport)
    #else
        #define SANDBOX_API __declspec(dllimport)
    #endif

    // Used exclusively by Plugins to expose their C-linkage entry points
    #define SANDBOX_EXPORT __declspec(dllexport)
    #define SANDBOX_COMPATIBLE_MODULE_EXTENSION ".dll"

#elif defined(SANDBOX_APPLE_PLATFORM)
    #define SANDBOX_API __attribute__((visibility("default")))
    #define SANDBOX_EXPORT __attribute__((visibility("default")))
    #define SANDBOX_COMPATIBLE_MODULE_EXTENSION ".dylib"

#elif defined(SANDBOX_LINUX_PLATFORM)
    #define SANDBOX_API __attribute__((visibility("default")))
    #define SANDBOX_EXPORT __attribute__((visibility("default")))
    #define SANDBOX_COMPATIBLE_MODULE_EXTENSION ".so"

#endif