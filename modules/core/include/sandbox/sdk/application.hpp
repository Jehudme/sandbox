#pragma once
#include <sandbox/services/application_service.h>

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
            return sandbox_application_is_running(entity_world.c_ptr());
        }
    };
}
#endif

