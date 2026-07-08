#include "configuration_module.h"
#include <sandbox/sdk/logs.hpp>
#include <flecs/addons/cpp/flecs.hpp>
#include <sandbox/sdk/configuration.hpp>

namespace sandbox::modules {
    configuration_module_t::configuration_module_t(flecs::world& entity_world) {
        sandbox::modules::logs::trace(entity_world, "Configuration Module Initializing...");
        
        // Register properties as a component on the world itself
        entity_world.set<sandbox::properties>(sandbox::properties());
        sandbox::properties& properties = entity_world.get_mut<sandbox::properties>();

        sandbox::modules::logs::trace(entity_world, "Discovering engine properties for configuration...");
        
        if (entity_world.entity("::sandbox::configuration::handle").has<uint64_t>()) {
            uint64_t token = entity_world.entity("::sandbox::configuration::handle").get<uint64_t>();
            sandbox_properties_handle_t engine_properties = { .token = token };
            if (SANDBOX_HANDLE_IS_VALID(engine_properties)) {
                sandbox::modules::logs::trace(entity_world, "Configuration Module Found engine properties. Merging...");
                sandbox_properties_merge(properties.get_raw(), "", engine_properties);
            }
        } else {
            sandbox::modules::logs::warn(entity_world, "Configuration handle not provided by the engine. Using default empty configuration.");
        }
    }
}


// ==========================================
namespace sandbox::modules {
    SANDBOX_DECLARE_MODULE(configuration_module_t, {
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
}
