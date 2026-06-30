#include "runtime.h"
#include <sandbox/sdk/logs.hpp>
#include <chrono>

namespace sandbox::modules {

    runtime_t::~runtime_t() {
        stop();
    }

    void runtime_t::main_loop(flecs::world& world) {
        sandbox::modules::logs::info(world, "Runtime loop started");

        while (world.progress()) {
            if (m_paused->load()) {
                std::unique_lock<std::mutex> lock(*m_mutex);
                sandbox::modules::logs::info(world, "Runtime paused");
                m_cv->wait(lock, [this]() { return !m_paused->load(); });
                sandbox::modules::logs::info(world, "Runtime resumed");
            }
        }
        
        sandbox::modules::logs::info(world, "Runtime loop stopped");
    }

    void runtime_t::run(flecs::world& world) {
        if (m_thread) {
            sandbox::modules::logs::warn(world, "Runtime is already running in a thread.");
            return;
        }
        main_loop(world);
    }

    void runtime_t::start(flecs::world& world) {
        if (m_thread) {
            sandbox::modules::logs::warn(world, "Runtime is already running.");
            return;
        }
        
        m_thread = std::make_shared<std::thread>([this, world]() mutable {
            main_loop(world);
        });
    }

    void runtime_t::stop() {
        if (m_thread && m_thread->joinable()) {
            m_thread->join();
            m_thread.reset();
        }
    }

    void runtime_t::pause() {
        m_paused->store(true);
    }

    void runtime_t::resume() {
        m_paused->store(false);
        m_cv->notify_all();
    }

}
