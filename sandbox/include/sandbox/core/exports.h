#pragma once

#include <flecs.h>
#include <string>

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
    #define SANDBOX_EXPORT __declspec(dllexport)
    #define SANDBOX_COMPATIBLE_MODULE_EXTENSION ".dll"
#else
    #define SANDBOX_EXPORT __attribute__((visibility("default")))
    #include <dlfcn.h>
    #include <cstring>
    #include <cstdint>
    #if defined(__APPLE__)
        #define SANDBOX_COMPATIBLE_MODULE_EXTENSION ".dylib"
    #else
        #define SANDBOX_COMPATIBLE_MODULE_EXTENSION ".so"
    #endif
#endif

#define SANDBOX_DEFINE_MODULE(ModuleClass, ModuleName)                         \
extern "C" SANDBOX_EXPORT void ModuleName##Import(ecs_world_t* raw_world) {    \
    flecs::world{raw_world}.import<ModuleClass>();                             \
}                                                                              \
extern "C" SANDBOX_EXPORT void ModuleName(ecs_world_t* raw_world) {            \
    flecs::world{raw_world}.import<ModuleClass>();                             \
}

#define SANDBOX_DEFINE_LIBRARY(MasterModuleClass)                              \
struct SandboxLibraryMain_Module {                                             \
    SandboxLibraryMain_Module(flecs::world& ecs_instance) {                    \
        ecs_instance.module<SandboxLibraryMain_Module>("SandboxLibraryMain");  \
        ecs_instance.import<MasterModuleClass>();                              \
    }                                                                          \
};                                                                             \
SANDBOX_DEFINE_MODULE(SandboxLibraryMain_Module, SandboxLibraryMain)

namespace sandbox {
    inline void configure_plugin_os_api() {
        ecs_os_set_api_defaults();

#if defined(__linux__) || defined(__APPLE__)
        ecs_os_api_t os_api = ecs_os_api;

        os_api.module_to_dl_ = [](const char* module) {
            std::string name = std::string(module) + SANDBOX_COMPATIBLE_MODULE_EXTENSION;
            char* result = static_cast<char*>(ecs_os_malloc(name.length() + 1));
            std::strcpy(result, name.c_str());
            return result;
        };

        os_api.dlopen_ = [](const char* lib) {
            return reinterpret_cast<uintptr_t>(dlopen(lib, RTLD_NOW | RTLD_GLOBAL));
        };

        os_api.dlproc_ = [](uintptr_t lib, const char* proc) {
            return reinterpret_cast<void(*)()>(dlsym(reinterpret_cast<void*>(lib), proc));
        };

        os_api.dlclose_ = [](uintptr_t lib) {
            dlclose(reinterpret_cast<void*>(lib));
        };

        ecs_os_set_api(&os_api);
#endif
    }
}