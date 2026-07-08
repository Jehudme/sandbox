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

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
namespace sandbox::modules {
    /**
     * @brief High-level C++ SDK for interacting with the application module.
     */
    class application {
    public:
        /**
         * @brief Checks if the application is currently running.
         * @param entity_world The flecs world.
         * @return True if running, false otherwise.
         */
        static bool is_running(const flecs::world& entity_world) {
            if (const auto* service = SANDBOX_GET_SERVICE(entity_world, sandbox_application_service_t)) {
                if (service->api && service->api->is_running) {
                    return service->api->is_running(entity_world.c_ptr());
                }
            }
            return false;
        }
    };
}
#endif
