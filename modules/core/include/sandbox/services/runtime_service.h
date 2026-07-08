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

#ifdef __cplusplus
}
#endif
