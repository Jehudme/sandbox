#pragma once
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

#ifdef __cplusplus
}
#endif
