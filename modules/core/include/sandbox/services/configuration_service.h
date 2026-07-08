#pragma once
#include <type_traits>
#include <vector>
#include <optional>
#include <string>
#include <flecs/addons/cpp/flecs.hpp>
#include <sandbox/sdk/properties.hpp>
#include <sandbox/abi/bootstrapper.h>
#include <sandbox/abi/properties.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief API for the configuration service.
 */
typedef struct {
    /**
     * @brief Retrieves the global properties handle.
     * @param ecs The entity component system world.
     * @return The global properties handle.
     */
    sandbox_properties_handle_t (*get_properties)(ecs_world_t* ecs);
} sandbox_configuration_api_t;

/**
 * @brief The configuration service definition for global properties.
 */
SANDBOX_DECLARE_SERVICE(sandbox_configuration_service_t, sandbox_configuration_api_t, {
    .name = "configuration",
    .description = "The configuration service for global properties",
    .architecture = "sandbox",
    .version_major = 1,
    .version_minor = 0
})

// --- Public C API ---
static inline sandbox_properties_handle_t sandbox_configuration_get_properties(ecs_world_t* ecs) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_configuration_service_t* service = flecs_world.try_get<sandbox_configuration_service_t>();
#else
    const sandbox_configuration_service_t* service = (const sandbox_configuration_service_t*)ecs_singleton_get(ecs, sandbox_configuration_service_t);
#endif
    if (service && service->api && service->api->get_properties) {
        return service->api->get_properties(ecs);
    }
    sandbox_properties_handle_t invalid = {0};
    return invalid;
}

#ifdef __cplusplus
}
#endif
