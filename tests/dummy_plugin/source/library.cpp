#include "sandbox/abi/dummy.h"
#include "sandbox/sdk/dummy.hpp"
#include <iostream>
#include <flecs.h>

#if defined(_WIN32) || defined(_WIN64)
#define DUMMY_EXPORT __declspec(dllexport)
#else
#define DUMMY_EXPORT __attribute__((visibility("default")))
#endif

extern "C" {
    // Keep this around for backwards compatibility with legacy tests
    DUMMY_EXPORT int sandbox_dummy_version() {
        return 1;
    }
}

// --- 1. Service API Implementation ---
static int dummy_get_magic_number() {
    std::cout << "[Dummy Plugin] Magic number requested!" << std::endl;
    return 42;
}

// Define the global API instance declared in dummy.h
sandbox_dummy_api_t g_dummy_api = {
    .get_magic_number = dummy_get_magic_number
};

// --- 2. C++ Module Implementation ---
namespace sandbox::modules {
    struct dummy_module_t {
        explicit dummy_module_t(flecs::world& world) {
            std::cout << "[Dummy Plugin] C++ Flecs module constructed!" << std::endl;
            
            // Example: Safely fetching our own service to verify it registered
            const sandbox_dummy_service_t* svc = SANDBOX_GET_SERVICE(world, sandbox_dummy_service_t);
            if (svc) {
                std::cout << "[Dummy Plugin] Verified dummy service is active! Magic: " 
                          << svc->api->get_magic_number() << std::endl;
            }
        }
    };
}

// --- 3. Module Declaration ---
// Alias the module so the macro can process it correctly
typedef sandbox::modules::dummy_module_t sandbox_dummy_module_t;

// The module declaration handles struct generation, Flecs importing, and 
// static DLL-load registration through sandbox_stage_module().
SANDBOX_DECLARE_MODULE(sandbox_dummy_module_t, {
    .name = "dummy",
    .description = "A professional dummy module",
    .architecture = "sandbox",
    .version_major = 1,
    .version_minor = 0,
    .version_patch = 0,
    .service = &sandbox_dummy_service_t_info, // Hooks up the service from dummy.h
    .requirements = nullptr,
    .requirement_count = 0
})
