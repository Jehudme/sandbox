// tests/dummy_plugin/dummy_plugin.cpp
#include <sandbox/abi/bootstrapper.h>
#include <iostream>

#if defined(_WIN32) || defined(_WIN64)
#define DUMMY_EXPORT __declspec(dllexport)
#else
#define DUMMY_EXPORT __attribute__((visibility("default")))
#endif

// We keep the old sandbox_dummy_version() just in case any test accidentally references it
extern "C" {
    DUMMY_EXPORT int sandbox_dummy_version() {
        return 1;
    }
}

// --- 1. The Module Initialization Callback ---
// This is the flat C callback that the engine will call during the Bootstrapper's boot phase.
static void dummy_module_init(ecs_world_t* ecs) {
    std::cout << "[Dummy Plugin] Init callback executed! World: " << ecs << std::endl;
    // Here you would register components, systems, etc., into the Flecs world.
}

// --- 2. The Module Metadata ---
// We fill out the static C struct required by the engine's ABI.
static sandbox_module_info_t g_dummy_info = {
    .name = "dummy_plugin",
    .description = "A simple example module plugin",
    .architecture = "x86_64", // Or whatever you standardize on
    .version_major = 1,
    .version_minor = 0,
    .version_patch = 0,
    .service = nullptr,       // No service exported by this module
    .requirements = nullptr,  // No strict requirements for this module
    .requirement_count = 0,
    .init_fn = dummy_module_init
};

// --- 3. The Registration Hook ---
// Because we are loaded dynamically via dlopen (in library_loader.cpp),
// we use a static C++ initializer to execute code the moment the DLL is mapped into memory.
// It calls back into the host engine to stage the module.
struct DummyModuleRegistrar {
    DummyModuleRegistrar() {
        std::cout << "[Dummy Plugin] DLL Loaded. Staging module..." << std::endl;
        sandbox_stage_module(&g_dummy_info);
    }
};

// This static instance guarantees the constructor runs on DLL load.
static DummyModuleRegistrar g_registrar_instance;
