#pragma once
#include <flecs.h>

#include <sandbox/abi/platform.h>
#include <sandbox/abi/bootstrapper.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The API for the runtime service.
 */
typedef struct sandbox_runtime_api_t {
    /**
     * @brief Runs the main loop blocking the current thread.
     * @param ecs The entity component system world.
     */
    void (*run)(ecs_world_t* ecs);
    /**
     * @brief Starts the runtime asynchronously.
     * @param ecs The entity component system world.
     */
    void (*start)(ecs_world_t* ecs);
    /**
     * @brief Stops the runtime.
     * @param ecs The entity component system world.
     */
    void (*stop)(ecs_world_t* ecs);
    /**
     * @brief Pauses the runtime.
     * @param ecs The entity component system world.
     */
    void (*pause)(ecs_world_t* ecs);
    /**
     * @brief Resumes the runtime.
     * @param ecs The entity component system world.
     */
    void (*resume)(ecs_world_t* ecs);
} sandbox_runtime_api_t;

/**
 * @brief The runtime service definition.
 */
SANDBOX_DECLARE_SERVICE(sandbox_runtime_service_t, sandbox_runtime_api_t, {
    .name = "runtime",
    .description = "Global runtime module service",
    .architecture = "sandbox",
    .version_major = 1,
    .version_minor = 0,
})

// --- Public C API ---
static inline void sandbox_runtime_run(ecs_world_t* ecs) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_runtime_service_t* service = flecs_world.try_get<sandbox_runtime_service_t>();
#else
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_runtime_service_t* service = flecs_world.try_get<sandbox_runtime_service_t>();
#else
    const sandbox_runtime_service_t* service = (const sandbox_runtime_service_t*)ecs_singleton_get(ecs, sandbox_runtime_service_t);
#endif
#endif
    if (service && service->api && service->api->run) {
        service->api->run(ecs);
        return;
    }
    
}
static inline void sandbox_runtime_start(ecs_world_t* ecs) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_runtime_service_t* service = flecs_world.try_get<sandbox_runtime_service_t>();
#else
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_runtime_service_t* service = flecs_world.try_get<sandbox_runtime_service_t>();
#else
    const sandbox_runtime_service_t* service = (const sandbox_runtime_service_t*)ecs_singleton_get(ecs, sandbox_runtime_service_t);
#endif
#endif
    if (service && service->api && service->api->start) {
        service->api->start(ecs);
        return;
    }
    
}
static inline void sandbox_runtime_stop(ecs_world_t* ecs) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_runtime_service_t* service = flecs_world.try_get<sandbox_runtime_service_t>();
#else
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_runtime_service_t* service = flecs_world.try_get<sandbox_runtime_service_t>();
#else
    const sandbox_runtime_service_t* service = (const sandbox_runtime_service_t*)ecs_singleton_get(ecs, sandbox_runtime_service_t);
#endif
#endif
    if (service && service->api && service->api->stop) {
        service->api->stop(ecs);
        return;
    }
    
}
static inline void sandbox_runtime_pause(ecs_world_t* ecs) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_runtime_service_t* service = flecs_world.try_get<sandbox_runtime_service_t>();
#else
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_runtime_service_t* service = flecs_world.try_get<sandbox_runtime_service_t>();
#else
    const sandbox_runtime_service_t* service = (const sandbox_runtime_service_t*)ecs_singleton_get(ecs, sandbox_runtime_service_t);
#endif
#endif
    if (service && service->api && service->api->pause) {
        service->api->pause(ecs);
        return;
    }
    
}
static inline void sandbox_runtime_resume(ecs_world_t* ecs) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_runtime_service_t* service = flecs_world.try_get<sandbox_runtime_service_t>();
#else
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_runtime_service_t* service = flecs_world.try_get<sandbox_runtime_service_t>();
#else
    const sandbox_runtime_service_t* service = (const sandbox_runtime_service_t*)ecs_singleton_get(ecs, sandbox_runtime_service_t);
#endif
#endif
    if (service && service->api && service->api->resume) {
        service->api->resume(ecs);
        return;
    }
    
}

#ifdef __cplusplus
}
#endif
