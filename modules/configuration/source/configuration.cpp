#include "configuration.h"
#include <iostream>
#include <flecs/addons/cpp/flecs.hpp>
#include "sandbox/sdk/configuration.hpp"

namespace sandbox::modules {
    configuration_module_t::configuration_module_t(flecs::world& world) {
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
}
