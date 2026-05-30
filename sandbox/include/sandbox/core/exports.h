#pragma once

#include <flecs.h>

#if defined(_WIN32) || defined(__CYGWIN__)
    #define SANDBOX_EXPORT __declspec(dllexport)
#else
    #define SANDBOX_EXPORT __attribute__((visibility("default")))
#endif

// For standard individual module files where you want a custom lookup name
#define SANDBOX_DEFINE_MODULE(ModuleClass, ModuleName)                         \
extern "C" SANDBOX_EXPORT void ModuleName##Import(ecs_world_t* raw_world) {\
flecs::world world(raw_world);                                         \
world.import<ModuleClass>();                                           \
}

// For your Master Library files. This creates a predictable, fixed engine entry point.
#define SANDBOX_DEFINE_LIBRARY(MasterModuleClass)                              \
SANDBOX_DEFINE_MODULE(MasterModuleClass, SandboxLibraryMain)

#if defined(_WIN32) || defined(_WIN64)
    #define SANDBOX_COMPATIBLE_MODULE_EXTENSION ".dll"
#elif defined(__APPLE__)
    #define SANDBOX_COMPATIBLE_MODULE_EXTENSION ".dylib"
#else
    #define SANDBOX_COMPATIBLE_MODULE_EXTENSION ".so"
#endif