#pragma once

#include "sandbox/core/platform.h"
#include "sandbox/core/bootstrapper.h"
#include "sandbox/core/module_info.h"
#include <flecs.h>

namespace sandbox::detail {

    inline void stage_library(ecs_world_t* raw_world) {
        flecs::world world{raw_world};

        world.get_mut<sandbox::bootstrapper>().stage(sandbox::get_local_registry());
    }

} // namespace sandbox::detail

// ============================================================================
// MODULE SELF-REGISTRATION MACRO
// ============================================================================

#define SANDBOX_DECLARE_MODULE(Class, Name, Major, Minor, Patch, Service, ...) \
    static inline bool Class##_registered = []() { \
        sandbox::get_local_registry().modules.push_back( \
            sandbox::create_module_info<Class>( \
                #Name, \
                Major, \
                Minor, \
                Patch, \
                std::vector<sandbox::requirement>{__VA_ARGS__}, \
                Service \
            ) \
        ); \
        return true; \
    }()

#define SANDBOX_DECLARE_SERVICE_CONTRACT(Name, Major, Minor) \
    static inline bool Name##_contract_registered = []() { \
        sandbox::get_local_registry().services.push_back( \
            sandbox::create_service_info( \
                #Name, \
                Major, \
                Minor \
            ) \
        ); \
        return true; \
    }()

// ============================================================================
// FLECS-COMPATIBLE DLL ENTRY POINT
// ============================================================================

#define SANDBOX_DECLARE_LIBRARY()                                               \
    /* 1. Define a dummy struct to satisfy Flecs' strict type rules */          \
    struct SandboxLibraryMain_Dummy {};                                         \
                                                                                \
    /* 2. The entry point Flecs searches for when using import_from_library */  \
    extern "C" SANDBOX_EXPORT void SandboxLibraryMain(ecs_world_t* world) {     \
        flecs::world ecs{world};                                                \
                                                                                \
        /* Trick Flecs into thinking the main library was imported */           \
        ecs.module<SandboxLibraryMain_Dummy>("SandboxLibraryMain");             \
                                                                                \
        /* Push the hidden data to the Engine's Bootstrapper */                 \
        sandbox::detail::stage_library(world);                                  \
    }

namespace sandbox {
    void configure_plugin_os_api();
}