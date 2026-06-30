#pragma once

#include <sandbox/abi/runtime.h>
#include <flecs.h>

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
