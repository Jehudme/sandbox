#include <sandbox/sdk/configuration.hpp>
#include <sandbox/sdk/logs.hpp>
#include "sandbox/services/configuration_service.h"
#include "configuration_module.h"
#include <flecs.h>

// C-ABI Endpoints
// ==========================================

static sandbox_properties_handle_t config_get_properties(ecs_world_t* entity_world);

sandbox_configuration_api_t g_configuration_api = {
    .get_properties = config_get_properties
};

SANDBOX_DEFINE_SERVICE(sandbox_configuration_service_t, sandbox_configuration_api_t, &g_configuration_api)

static sandbox_properties_handle_t config_get_properties(ecs_world_t* entity_world) {
    if (!entity_world) return {0};
    flecs::world flecs_world(entity_world);
    if (flecs_world.has<sandbox::properties>()) {
        sandbox::properties& properties = flecs_world.get_mut<sandbox::properties>();
        return properties.get_raw();
    }
    return {0};
}

// --- Public C API Implementations ---
sandbox_properties_handle_t sandbox_configuration_get_properties(ecs_world_t* ecs) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_configuration_service_t* service = flecs_world.try_get<sandbox_configuration_service_t>();
#else
    const sandbox_configuration_service_t* service = (const sandbox_configuration_service_t*)ecs_singleton_get(ecs, sandbox_configuration_service_t);
#endif
    if (service && service->api && service->api->get_properties) {
        return service->api->get_properties(ecs);
    } else {
        sandbox::modules::logs::error(flecs_world, "[Configuration Module] Service not initialized!");
    }
    sandbox_properties_handle_t invalid = {0};
    return invalid;
}

// --- SDK Implementations ---
namespace sandbox::modules {
sandbox::properties configuration::get_properties(flecs::world& entity_world) {
            return sandbox::properties(sandbox_configuration_get_properties(entity_world.c_ptr()), false);
        }
}
