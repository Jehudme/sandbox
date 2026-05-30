#pragma once

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
    #define SANDBOX_EXPORT __declspec(dllexport)
    #define SANDBOX_COMPATIBLE_MODULE_EXTENSION ".dll"
#else
    #define SANDBOX_EXPORT __attribute__((visibility("default")))
    #if defined(__APPLE__)
        #define SANDBOX_COMPATIBLE_MODULE_EXTENSION ".dylib"
    #else
        #define SANDBOX_COMPATIBLE_MODULE_EXTENSION ".so"
    #endif
#endif