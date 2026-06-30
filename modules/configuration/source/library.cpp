// modules/configuration/source/library.cpp
#include "sandbox/abi/configuration.h"
#include "sandbox/sdk/configuration.hpp"
#include <flecs.h>
#include <iostream>

#include "configuration.h"
typedef sandbox::modules::configuration_module_t sandbox_configuration_module_t;

static sandbox_properties_handle_t config_get_properties(ecs_world_t* ecs);

sandbox_configuration_api_t g_configuration_api = {
    .get_properties = config_get_properties
};

SANDBOX_DEFINE_SERVICE(sandbox_configuration_service_t, sandbox_configuration_api_t, &g_configuration_api)



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
