#include "subsystems/runner/runner.h"

#include "sandbox/event_bus/event_bus.h"
#include "sandbox/event_bus/logger_events.h"
#include "sandbox/utilities/properties.h"

namespace sandbox::modules {

    runner::runner(world& ecs) {
        ecs.module<runner>("::Modules::Runner");
        ecs.set<sandbox::runner_service>({this});
    }

    runner::~runner() {
        quit();

        if (m_worker_thread.joinable()) {
            m_worker_thread.join();
        }
    }

    void runner::start_async(world& ecs) {
        std::lock_guard<std::mutex> lock(m_state_mutex);
        if (m_state != execution_state::Idle) return;

        m_state = execution_state::Running;
        SANDBOX_INFO(ecs, "[Runner] Starting async pipeline thread.");

        m_worker_thread = std::thread(&runner::internal_tick_loop, this, std::ref(ecs));
    }

    void runner::run_sync(world& ecs) {
        {
            std::lock_guard<std::mutex> lock(m_state_mutex);
            if (m_state != execution_state::Idle) return;
            m_state = execution_state::Running;
        }

        SANDBOX_INFO(ecs, "[Runner] Entering main execution loop on the calling thread.");
        internal_tick_loop(ecs);
    }

    void runner::quit() {
        std::lock_guard<std::mutex> lock(m_state_mutex);
        if (m_state == execution_state::Quitting) return;

        m_state = execution_state::Quitting;
        m_state_cv.notify_all();
    }

    void runner::pause() {
        std::lock_guard<std::mutex> lock(m_state_mutex);
        if (m_state == execution_state::Running) {
            m_state = execution_state::Paused;
        }
    }

    void runner::resume() {
        std::lock_guard<std::mutex> lock(m_state_mutex);
        if (m_state == execution_state::Paused) {
            m_state = execution_state::Running;
            m_state_cv.notify_all();
        }
    }

    void runner::internal_tick_loop(world& ecs) {
        ecs.set_target_fps(60);

        while (true) {
            {
                std::unique_lock<std::mutex> lock(m_state_mutex);
                m_state_cv.wait(lock, [this]() {
                    return m_state == execution_state::Running || m_state == execution_state::Quitting;
                });

                if (m_state == execution_state::Quitting) {
                    break;
                }
            }

            if (!ecs.progress()) {
                quit();
            }
        }
    }

} // namespace sandbox::modules
