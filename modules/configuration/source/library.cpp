// modules/configuration/source/library.cpp
#include "sandbox/abi/configuration.h"
#include "sandbox/sdk/configuration.hpp"
#include <flecs.h>
#include <iostream>

#if defined(_WIN32) || defined(_WIN64)
#define CONFIG_EXPORT __declspec(dllexport)
#else
#define CONFIG_EXPORT __attribute__((visibility("default")))
#endif

// We need an owned handle that persists for the lifetime of the plugin.
static sandbox_properties_handle_t g_properties_handle = {0};

static sandbox_properties_handle_t config_get_properties() {
    return g_properties_handle;
}

sandbox_configuration_api_t g_configuration_api = {
    .get_properties = config_get_properties
};

SANDBOX_DEFINE_SERVICE(sandbox_configuration_service_t, sandbox_configuration_api_t, &g_configuration_api, {
    .name = "configuration",
    .description = "The configuration service for global properties",
    .architecture = "sandbox",
    .version_major = 1,
    .version_minor = 0
})

namespace sandbox::modules {
    struct configuration_module_t {
        explicit configuration_module_t(flecs::world& world) {
            std::cout << "[Configuration Module] Initializing..." << std::endl;
            
            // Create our own properties handle to store the config safely
            g_properties_handle = sandbox_properties_create();

            std::cout << "[Configuration Module] Lookup entity: " << world.entity("::sandbox::configuration::handle").id() 
                      << " with uint64_t comp id: " << world.component<uint64_t>().id() << std::endl;
            if (world.entity("::sandbox::configuration::handle").has<uint64_t>()) {
                uint64_t token = world.entity("::sandbox::configuration::handle").get<uint64_t>();
                sandbox_properties_handle_t engine_props_handle = { .token = token };
                if (SANDBOX_HANDLE_IS_VALID(engine_props_handle)) {
                    std::cout << "[Configuration Module] Found engine properties. Merging..." << std::endl;
                    sandbox_properties_merge(g_properties_handle, "", engine_props_handle);
                }
            } else {
                std::cout << "[Configuration Module] Entity doesn't have uint64_t component!" << std::endl;
            }
        }
        
        ~configuration_module_t() {
            if (SANDBOX_HANDLE_IS_VALID(g_properties_handle)) {
                sandbox_properties_destroy(&g_properties_handle);
            }
        }
    };
}

typedef sandbox::modules::configuration_module_t sandbox_configuration_module_t;

SANDBOX_DECLARE_MODULE(sandbox_configuration_module_t, {
    .name = "configuration",
    .description = "Global configuration module",
    .architecture = "sandbox",
    .version_major = 1,
    .version_minor = 0,
    .version_patch = 0,
    .service = &sandbox_configuration_service_t_info,
    .requirements = nullptr,
    .requirement_count = 0
})
