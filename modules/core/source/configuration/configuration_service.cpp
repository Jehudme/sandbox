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
