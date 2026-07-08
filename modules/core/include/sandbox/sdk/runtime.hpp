#pragma once
#include <sandbox/services/runtime_service.h>

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
    static void run(flecs::world& entity_world);

        /**
         * @brief Starts the engine loop asynchronously in a background thread.
         * @param entity_world The flecs world.
         */
    static void start(flecs::world& entity_world);

        /**
         * @brief Stops the engine loop.
         * @param entity_world The flecs world.
         */
    static void stop(flecs::world& entity_world);

        /**
         * @brief Pauses the engine loop.
         * @param entity_world The flecs world.
         */
    static void pause(flecs::world& entity_world);

        /**
         * @brief Resumes the engine loop.
         * @param entity_world The flecs world.
         */
    static void resume(flecs::world& entity_world);
    };
}
#endif

