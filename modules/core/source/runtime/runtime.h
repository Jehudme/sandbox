#pragma once

#include "flecs.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace sandbox::modules {
    struct runtime_t {
        runtime_t() = default;
        ~runtime_t();

        void run(flecs::world& world);
        void start(flecs::world& world);
        void stop();
        void pause();
        void resume();

    private:
        void main_loop(flecs::world& world);

        std::shared_ptr<std::thread> m_thread;
        std::shared_ptr<std::mutex> m_mutex = std::make_shared<std::mutex>();
        std::shared_ptr<std::condition_variable> m_cv = std::make_shared<std::condition_variable>();
        std::shared_ptr<std::atomic<bool>> m_paused = std::make_shared<std::atomic<bool>>(false);
    };
}