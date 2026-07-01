#pragma once

#include <sandbox/abi/application.h>
#include <flecs.h>

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
