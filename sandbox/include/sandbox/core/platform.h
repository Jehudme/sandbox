#pragma once

// ============================================================================
// 1. Explicit Platform Detection
// ============================================================================

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
    #define SANDBOX_WINDOWS_PLATFORM

#elif defined(__APPLE__) && defined(__MACH__)
    #define SANDBOX_APPLE_PLATFORM

#elif defined(__linux__)
    #define SANDBOX_LINUX_PLATFORM

#else
    #error "[Sandbox Engine] Compilation failed: Unsupported or unknown platform detected!"
#endif


// ============================================================================
// 2. Platform-Specific Export & File Configuration
// ============================================================================

#if defined(SANDBOX_WINDOWS_PLATFORM)
    #define SANDBOX_EXPORT __declspec(dllexport)
    #define SANDBOX_COMPATIBLE_MODULE_EXTENSION ".dll"

#elif defined(SANDBOX_APPLE_PLATFORM)
    #define SANDBOX_EXPORT __attribute__((visibility("default")))
    #define SANDBOX_COMPATIBLE_MODULE_EXTENSION ".dylib"

#elif defined(SANDBOX_LINUX_PLATFORM)
    #define SANDBOX_EXPORT __attribute__((visibility("default")))
    #define SANDBOX_COMPATIBLE_MODULE_EXTENSION ".so"

#endif