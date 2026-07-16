#pragma once

#include "flecs.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace sandbox::modules {
    /**
     * @brief A global module that manages the engine's runtime loop execution.
     */
    struct runtime_t {
        /**
         * @brief Constructs a new runtime instance.
         */
/**
         * @brief Default constructor.
         */
        runtime_t() = default;
        /**
         * @brief Destroys the runtime instance and joins any running threads.
         */
/**
         * @brief Destructor.
         */
        ~runtime_t();

        /**
         * @brief Runs the engine loop synchronously on the current thread.
         * @param entity_world The flecs world.
         */
/**
         * @brief Runs the main loop on the current thread.
         * @param entity_world The flecs world.
         */
        void run(flecs::world& entity_world);
        /**
         * @brief Starts the engine loop asynchronously in a background thread.
         * @param entity_world The flecs world.
         */
/**
         * @brief Starts the runtime asynchronously.
         * @param entity_world The flecs world.
         */
        void start(flecs::world& entity_world);
        /**
         * @brief Stops the engine loop.
         */
/**
         * @brief Stops the runtime.
         * @param entity_world The flecs world.
         */
        void stop(flecs::world& entity_world);
        /**
         * @brief Pauses the engine loop.
         */
/**
         * @brief Pauses the runtime.
         */
        void pause();
        /**
         * @brief Resumes the engine loop.
         */
/**
         * @brief Resumes the runtime.
         */
        void resume();

    private:
/**
         * @brief The internal main loop.
         * @param entity_world The flecs world.
         */
        void main_loop(flecs::world& entity_world);

        std::shared_ptr<std::thread> m_thread;
        std::shared_ptr<std::mutex> m_mutex = std::make_shared<std::mutex>();
        std::shared_ptr<std::condition_variable> m_cv = std::make_shared<std::condition_variable>();
        std::shared_ptr<std::atomic<bool>> m_paused = std::make_shared<std::atomic<bool>>(false);
        std::shared_ptr<std::atomic<bool>> m_running = std::make_shared<std::atomic<bool>>(false);
    };
}