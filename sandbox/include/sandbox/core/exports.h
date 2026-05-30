#pragma once

#include <flecs.h>
#include <string>
#include <cstdint> // Required for uintptr_t

#if defined(__linux__) || defined(__APPLE__)
    #include <dlfcn.h>
    #include <cstring>
#endif

#if defined(_WIN32) || defined(__CYGWIN__)
    #define SANDBOX_EXPORT __declspec(dllexport)
#else
    #define SANDBOX_EXPORT __attribute__((visibility("default")))
#endif

#if defined(_WIN32) || defined(_WIN64)
    #define SANDBOX_COMPATIBLE_MODULE_EXTENSION ".dll"
#elif defined(__APPLE__)
    #define SANDBOX_COMPATIBLE_MODULE_EXTENSION ".dylib"
#else
    #define SANDBOX_COMPATIBLE_MODULE_EXTENSION ".so"
#endif

// ----------------------------------------------------------------------------
// 1. Standard Module Export
// ----------------------------------------------------------------------------
#define SANDBOX_DEFINE_MODULE(ModuleClass, ModuleName)                         \
extern "C" SANDBOX_EXPORT void ModuleName##Import(ecs_world_t* raw_world) {    \
    flecs::world world(raw_world);                                             \
    world.import<ModuleClass>();                                               \
}                                                                              \
extern "C" SANDBOX_EXPORT void ModuleName(ecs_world_t* raw_world) {            \
    flecs::world world(raw_world);                                             \
    world.import<ModuleClass>();                                               \
}

// ----------------------------------------------------------------------------
// 2. Master Library Export (The Wrapper Fix)
// ----------------------------------------------------------------------------
// This creates a native struct with a unique type name, registers it using the
// exact string Flecs asserts against, and delegates to your MasterModuleClass!
#define SANDBOX_DEFINE_LIBRARY(MasterModuleClass)                              \
struct SandboxLibraryMain_Module {                                             \
    SandboxLibraryMain_Module(flecs::world& ecs_instance) {                    \
        ecs_instance.module<SandboxLibraryMain_Module>("SandboxLibraryMain");  \
        ecs_instance.import<MasterModuleClass>();                              \
    }                                                                          \
};                                                                             \
SANDBOX_DEFINE_MODULE(SandboxLibraryMain_Module, SandboxLibraryMain)


namespace sandbox {

    /**
     * @brief Configures Flecs to support dynamic library loading on all operating systems.
     * Automatically handles POSIX bindings and extension appending for Linux/Mac.
     */
    inline void configure_plugin_os_api() {
        ecs_os_set_api_defaults();

#if defined(__linux__) || defined(__APPLE__)
        ecs_os_api_t os_api = ecs_os_api;

        os_api.module_to_dl_ = [](const char* module) {
            std::string name = std::string(module) + SANDBOX_COMPATIBLE_MODULE_EXTENSION;
            char* result = (char*)ecs_os_malloc(name.length() + 1);
            std::strcpy(result, name.c_str());
            return result;
        };

        // 1. dlopen returns void*, cast it to an integer (uintptr_t)
        os_api.dlopen_ = [](const char* lib) -> uintptr_t {
            return reinterpret_cast<uintptr_t>(dlopen(lib, RTLD_NOW | RTLD_GLOBAL));
        };

        // 2. dlproc takes the integer handle, casts it back to void* for dlsym
        os_api.dlproc_ = [](uintptr_t lib, const char* proc) -> void(*)() {
            return reinterpret_cast<void(*)()>(dlsym(reinterpret_cast<void*>(lib), proc));
        };

        // 3. dlclose takes the integer handle and casts it back to void*
        os_api.dlclose_ = [](uintptr_t lib) {
            dlclose(reinterpret_cast<void*>(lib));
        };

        ecs_os_set_api(&os_api);
#endif
    }

} // namespace sandbox