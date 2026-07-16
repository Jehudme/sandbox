#include "runtime_module.h"
#include <sandbox/sdk/runtime.hpp>
#include <sandbox/sdk/logs.hpp>
#include <chrono>

namespace sandbox::modules {

    runtime_t::~runtime_t() {
        m_running->store(false);
        resume();
        if (m_thread && m_thread->joinable()) {
            m_thread->join();
        }
    }

    void runtime_t::main_loop(flecs::world& entity_world) {
        sandbox::modules::logs::info(entity_world, "Runtime loop started");

        while (m_running->load() && entity_world.progress()) {
            if (m_paused->load()) {
                std::unique_lock<std::mutex> lock(*m_mutex);
                sandbox::modules::logs::info(entity_world, "Runtime paused");
                m_cv->wait(lock, [this]() { return !m_paused->load() || !m_running->load(); });
                sandbox::modules::logs::info(entity_world, "Runtime resumed");
            }
        }
        
        sandbox::modules::logs::info(entity_world, "Runtime loop stopped");
    }

    void runtime_t::run(flecs::world& entity_world) {
        if (m_thread) {
            sandbox::modules::logs::warn(entity_world, "Runtime is already running in a thread.");
            return;
        }
        m_running->store(true);
        main_loop(entity_world);
    }

    void runtime_t::start(flecs::world& entity_world) {
        if (m_thread) {
            sandbox::modules::logs::warn(entity_world, "Runtime is already running.");
            return;
        }
        
        m_running->store(true);
        m_thread = std::make_shared<std::thread>([this, entity_world]() mutable {
            main_loop(entity_world);
        });
    }

    void runtime_t::stop(flecs::world& entity_world) {
        m_running->store(false);
        entity_world.quit();
        resume(); // This notifies the condition variable in case it was paused
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


// ==========================================
static sandbox_requirement_info_t runtime_requirements[] = {
    {
        .kind = SANDBOX_REQUIREMENT_KIND_SERVICE,
        .strictness = SANDBOX_REQUIREMENT_STRICTNESS_REQUIRED,
        .name = "logs",
        .architecture = "sandbox",
        .version_major = 1,
        .version_minor = 0,
        .version_patch = -1
    }
};

namespace sandbox::modules {
    struct runtime_module_t {
        runtime_module_t(flecs::world& entity_world) {
            entity_world.component<runtime_t>();
            entity_world.set<runtime_t>(runtime_t());
        }
    };

    SANDBOX_DECLARE_MODULE(runtime_module_t, {
        .name = "runtime",
        .description = "Global runtime module",
        .architecture = "sandbox",
        .version_major = 1,
        .version_minor = 0,
        .version_patch = 0,
        .service = &sandbox_runtime_service_t_info,
        .requirements = runtime_requirements,
        .requirement_count = 1
    })
}
