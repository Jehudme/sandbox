// modules/configuration/source/library.cpp
#include "sandbox/abi/configuration.h"
#include "sandbox/sdk/configuration.hpp"
#include <flecs.h>
#include <iostream>

// We need a forward declaration of the module
namespace sandbox::modules {
    struct configuration_module_t;
}
typedef sandbox::modules::configuration_module_t sandbox_configuration_module_t;

static sandbox_properties_handle_t config_get_properties(ecs_world_t* ecs);

sandbox_configuration_api_t g_configuration_api = {
    .get_properties = config_get_properties
};

SANDBOX_DEFINE_SERVICE(sandbox_configuration_service_t, sandbox_configuration_api_t, &g_configuration_api)

namespace sandbox::modules {
    struct configuration_module_t {
        explicit configuration_module_t(flecs::world& world) {
            std::cout << "[Configuration Module] Initializing..." << std::endl;
            
            // Register properties as a component on the world itself
            world.set<sandbox::properties>(sandbox::properties());
            sandbox::properties& props = world.get_mut<sandbox::properties>();

            std::cout << "[Configuration Module] Lookup entity: " << world.entity("::sandbox::configuration::handle").id() 
                      << " with uint64_t comp id: " << world.component<uint64_t>().id() << std::endl;
            if (world.entity("::sandbox::configuration::handle").has<uint64_t>()) {
                uint64_t token = world.entity("::sandbox::configuration::handle").get<uint64_t>();
                sandbox_properties_handle_t engine_props_handle = { .token = token };
                if (SANDBOX_HANDLE_IS_VALID(engine_props_handle)) {
                    std::cout << "[Configuration Module] Found engine properties. Merging..." << std::endl;
                    sandbox_properties_merge(props.get_raw(), "", engine_props_handle);
                }
            } else {
                std::cout << "[Configuration Module] Entity doesn't have uint64_t component!" << std::endl;
            }
        }
    };
}

static sandbox_properties_handle_t config_get_properties(ecs_world_t* ecs) {
    if (!ecs) return {0};
    flecs::world world(ecs);
    if (world.has<sandbox::properties>()) {
        sandbox::properties& props = world.get_mut<sandbox::properties>();
        return props.get_raw();
    }
    return {0};
}

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
