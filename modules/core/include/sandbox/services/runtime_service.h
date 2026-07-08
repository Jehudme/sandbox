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

#ifdef __cplusplus
namespace sandbox::modules {
    /**
     * @brief High-level C++ SDK for interacting with the runtime module.
     */
    class runtime {
    public:
        /**
         * @brief Runs the engine loop synchronously on the current thread.
         * @param entity_world The flecs world.
         */
        static void run(flecs::world& entity_world) {
            if (const auto* service = SANDBOX_GET_SERVICE(entity_world, sandbox_runtime_service_t)) {
                if (service->api && service->api->run) {
                    service->api->run(entity_world.c_ptr());
                }
            }
        }

        /**
         * @brief Starts the engine loop asynchronously in a background thread.
         * @param entity_world The flecs world.
         */
        static void start(flecs::world& entity_world) {
            if (const auto* service = SANDBOX_GET_SERVICE(entity_world, sandbox_runtime_service_t)) {
                if (service->api && service->api->start) {
                    service->api->start(entity_world.c_ptr());
                }
            }
        }

        /**
         * @brief Stops the engine loop.
         * @param entity_world The flecs world.
         */
        static void stop(flecs::world& entity_world) {
            if (const auto* service = SANDBOX_GET_SERVICE(entity_world, sandbox_runtime_service_t)) {
                if (service->api && service->api->stop) {
                    service->api->stop(entity_world.c_ptr());
                }
            }
        }

        /**
         * @brief Pauses the engine loop.
         * @param entity_world The flecs world.
         */
        static void pause(flecs::world& entity_world) {
            if (const auto* service = SANDBOX_GET_SERVICE(entity_world, sandbox_runtime_service_t)) {
                if (service->api && service->api->pause) {
                    service->api->pause(entity_world.c_ptr());
                }
            }
        }

        /**
         * @brief Resumes the engine loop.
         * @param entity_world The flecs world.
         */
        static void resume(flecs::world& entity_world) {
            if (const auto* service = SANDBOX_GET_SERVICE(entity_world, sandbox_runtime_service_t)) {
                if (service->api && service->api->resume) {
                    service->api->resume(entity_world.c_ptr());
                }
            }
        }
    };
}
#endif
