#include "configuration.h"
#include <sandbox/sdk/logs.hpp>
#include <flecs/addons/cpp/flecs.hpp>
#include "sandbox/sdk/configuration.hpp"

namespace sandbox::modules {
    configuration_module_t::configuration_module_t(flecs::world& world) {
        sandbox::modules::logs::trace(world, "Configuration Module Initializing...");
        
        // Register properties as a component on the world itself
        world.set<sandbox::properties>(sandbox::properties());
        sandbox::properties& props = world.get_mut<sandbox::properties>();

        sandbox::modules::logs::trace(world, "Discovering engine properties for configuration...");
        
        if (world.entity("::sandbox::configuration::handle").has<uint64_t>()) {
            uint64_t token = world.entity("::sandbox::configuration::handle").get<uint64_t>();
            sandbox_properties_handle_t engine_props_handle = { .token = token };
            if (SANDBOX_HANDLE_IS_VALID(engine_props_handle)) {
                sandbox::modules::logs::trace(world, "Configuration Module Found engine properties. Merging...");
                sandbox_properties_merge(props.get_raw(), "", engine_props_handle);
            }
        } else {
            sandbox::modules::logs::warn(world, "Configuration handle not provided by the engine. Using default empty configuration.");
        }
    }
}
