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
        static void run(flecs::world& entity_world) {
            sandbox_runtime_run(entity_world.c_ptr());}

        /**
         * @brief Starts the engine loop asynchronously in a background thread.
         * @param entity_world The flecs world.
         */
        static void start(flecs::world& entity_world) {
            sandbox_runtime_start(entity_world.c_ptr());}

        /**
         * @brief Stops the engine loop.
         * @param entity_world The flecs world.
         */
        static void stop(flecs::world& entity_world) {
            sandbox_runtime_stop(entity_world.c_ptr());}

        /**
         * @brief Pauses the engine loop.
         * @param entity_world The flecs world.
         */
        static void pause(flecs::world& entity_world) {
            sandbox_runtime_pause(entity_world.c_ptr());}

        /**
         * @brief Resumes the engine loop.
         * @param entity_world The flecs world.
         */
        static void resume(flecs::world& entity_world) {
            sandbox_runtime_resume(entity_world.c_ptr());}
    };
}
#endif

