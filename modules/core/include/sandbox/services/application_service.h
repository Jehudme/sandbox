#pragma once
#include <flecs.h>
#include <sandbox/abi/bootstrapper.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sandbox_application_api_t {
    bool (*is_running)(ecs_world_t* ecs);
} sandbox_application_api_t;

SANDBOX_DECLARE_SERVICE(sandbox_application_service_t, sandbox_application_api_t, {
    .struct_size = 0,
    .name = "application",
    .description = "Application lifecycle service",
    .architecture = "sandbox",
    .version_major = 1,
    .version_minor = 0,
    .init_fn = NULL
});

// --- Public C API ---
static inline bool sandbox_application_is_running(ecs_world_t* ecs) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_application_service_t* service = flecs_world.try_get<sandbox_application_service_t>();
#else
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_application_service_t* service = flecs_world.try_get<sandbox_application_service_t>();
#else
    const sandbox_application_service_t* service = (const sandbox_application_service_t*)ecs_singleton_get(ecs, sandbox_application_service_t);
#endif
#endif
    if (service && service->api && service->api->is_running) {
        return service->api->is_running(ecs);
        
    }
    return false;
}

#ifdef __cplusplus
}
#endif
