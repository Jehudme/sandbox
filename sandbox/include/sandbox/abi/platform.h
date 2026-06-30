#pragma once

/**
 * @brief ==========================================================================
 */
/**
 * @brief PLATFORM DETECTION
 */
/**
 * @brief ==========================================================================
 */

#if defined(_WIN32) || defined(_WIN64)
    #define SANDBOX_PLATFORM_WINDOWS
#elif defined(__APPLE__) && defined(__MACH__)
    #define SANDBOX_PLATFORM_MACOS
#elif defined(__linux__)
    #define SANDBOX_PLATFORM_LINUX
#else
    #error "Unknown platform!"
#endif

/**
 * @brief ==========================================================================
 */
/**
 * @brief COMPILER DETECTION & INLINING
 */
/**
 * @brief ==========================================================================
 */

#if defined(__clang__)
    #define SANDBOX_COMPILER_CLANG
    #define SANDBOX_FORCE_INLINE inline __attribute__((always_inline))
#elif defined(__GNUC__)
    #define SANDBOX_COMPILER_GCC
    #define SANDBOX_FORCE_INLINE inline __attribute__((always_inline))
#elif defined(_MSC_VER)
    #define SANDBOX_COMPILER_MSVC
    #define SANDBOX_FORCE_INLINE __forceinline
#else
    #define SANDBOX_COMPILER_UNKNOWN
    #define SANDBOX_FORCE_INLINE inline
#endif

/**
 * @brief ==========================================================================
 */
/**
 * @brief SHARED LIBRARY EXPORT / IMPORT (SANDBOX_API)
 */
/**
 * @brief ==========================================================================
 */

#ifdef SANDBOX_STATIC
    /**
 * @brief Static build: No export/import needed
 */
    #define SANDBOX_API
#else
    /**
 * @brief Dynamic build (DLL / .so)
 */
    #ifdef SANDBOX_PLATFORM_WINDOWS
        #ifdef SANDBOX_BUILD_SHARED
            /**
 * @brief We are building the engine/library itself
 */
            #define SANDBOX_API __declspec(dllexport)
        #else
            /**
 * @brief We are a plugin/application importing the library
 */
            #define SANDBOX_API __declspec(dllimport)
        #endif
    #else
        /**
 * @brief Linux and macOS
 */
        #ifdef SANDBOX_BUILD_SHARED
            #define SANDBOX_API __attribute__((visibility("default")))
        #else
            #define SANDBOX_API
        #endif
    #endif
#endif

/**
 * @brief ==========================================================================
 */
/**
 * @brief COMPILER-SPECIFIC AUTO-INITIALIZATION MACROS
 */
/**
 * @brief ==========================================================================
 */

#if defined(SANDBOX_COMPILER_GCC) || defined(SANDBOX_COMPILER_CLANG)
    /**
 * @brief GCC / Clang (Linux, macOS, MinGW)
 */
    #define SANDBOX_CONSTRUCTOR(fn_name) \
        static void __attribute__((constructor)) fn_name(void)

#elif defined(SANDBOX_COMPILER_MSVC)
    /**
 * @brief Microsoft Visual Studio (Windows)
 */
    /**
 * @brief We inject a function pointer directly into the C-Runtime (CRT) initialization section
 */
    #pragma section(".CRT$XCU", read)
    #define SANDBOX_CONSTRUCTOR(fn_name) \
        static void __cdecl fn_name(void); \
        __declspec(allocate(".CRT$XCU")) void (__cdecl *fn_name##_)(void) = fn_name; \
        static void __cdecl fn_name(void)

#else
    /**
 * @brief Fallback for completely unknown compilers (Embedded systems, custom compilers)
 */
    /**
 * @brief It will compile, but the engine must manually call the function later
 */
    #pragma message("Warning: Compiler does not support C-constructors. Manual init required.")
    #define SANDBOX_CONSTRUCTOR(fn_name) \
        static void fn_name(void)
#endif

/**
 * @brief ==========================================================================
 */
/**
 * @brief DEBUG BREAK (CRASH/ASSERT HOOKS)
 */
/**
 * @brief ==========================================================================
 */

#if defined(SANDBOX_COMPILER_MSVC)
    #define SANDBOX_DEBUGBREAK() __debugbreak()
#elif defined(SANDBOX_COMPILER_GCC) || defined(SANDBOX_COMPILER_CLANG)
    #if defined(__i386__) || defined(__x86_64__)
        #define SANDBOX_DEBUGBREAK() __asm__ volatile("int $0x03")
    #elif defined(__aarch64__)
        #define SANDBOX_DEBUGBREAK() __asm__ volatile("brk #0")
    #else
        #define SANDBOX_DEBUGBREAK() __builtin_trap()
    #endif
#else
    #include <stdlib.h>
    #define SANDBOX_DEBUGBREAK() abort()
#endif