#include "modules/runner.h"

#include "sandbox/utilities/events.h"
#include "sandbox/macros/logger.h"
#include "sandbox/utilities/properties.h"

namespace sandbox::modules {

    runner::runner(world& ecs) {
        ecs.module<runner>("::Modules::Runner");

        // Subscribe to the state_change event so external systems/UI can control the engine
        sandbox::events::subscribe<events::runner::state_change>(
            ecs,
            [this, &ecs](const events::runner::state_change& event) {
                switch (event.state_request) {
                    case events::runner::state_change::action::Start:  start_async(ecs); break;
                    case events::runner::state_change::action::Run:    run_sync(ecs);    break;
                    case events::runner::state_change::action::Quit:   quit();           break;
                    case events::runner::state_change::action::Pause:  pause();          break;
                    case events::runner::state_change::action::Resume: resume();         break;
                }
            }
        );

        SANDBOX_INFO(ecs, "[Runner] Mounted and listening for state events.");
    }

    runner::~runner() {
        quit(); // Guarantee the engine stops when the module is destroyed

        // If we spawned an async background thread, wait for it to finish gracefully
        if (m_worker_thread.joinable()) {
            m_worker_thread.join();
        }
    }

    void runner::start_async(world& ecs) {
        std::lock_guard<std::mutex> lock(m_state_mutex);
        if (m_state != execution_state::Idle) return;

        m_state = execution_state::Running;
        SANDBOX_INFO(ecs, "[Runner] Launching async background thread.");

        m_worker_thread = std::thread(&runner::internal_tick_loop, this, std::ref(ecs));
    }

    void runner::run_sync(world& ecs) {
        {
            std::lock_guard<std::mutex> lock(m_state_mutex);
            if (m_state != execution_state::Idle) return;
            m_state = execution_state::Running;
        }

        SANDBOX_INFO(ecs, "[Runner] Starting blocking execution loop.");
        internal_tick_loop(ecs);
    }

    void runner::quit() {
        std::lock_guard<std::mutex> lock(m_state_mutex);
        if (m_state == execution_state::Quitting) return;

        m_state = execution_state::Quitting;
        m_state_cv.notify_all(); // Wake up the thread immediately if it was paused

        // Note: Cannot use SANDBOX_INFO here reliably because quit() is called from destructor
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
            m_state_cv.notify_all(); // Wake the sleeping loop thread
        }
    }

    void runner::internal_tick_loop(world& ecs) {
        properties manifest = ecs.lookup("::manifest").get<properties>();
        float target_fps = manifest.get<float>({"engine", "target_fps"}).value_or(60.0f);
        ecs.set_target_fps(target_fps);

        while (true) {
            // 1. Synchronize State
            {
                std::unique_lock<std::mutex> lock(m_state_mutex);

                // If paused, put this thread to sleep until a resume or quit wakes it up.
                // This drops CPU usage for this thread down to 0%.
                m_state_cv.wait(lock, [this]() {
                    return m_state == execution_state::Running || m_state == execution_state::Quitting;
                });

                if (m_state == execution_state::Quitting) {
                    break; // Exit the while loop cleanly
                }
            }

            // 2. Execute Engine Frame
            if (!ecs.progress()) {
                quit(); // If Flecs internally signals a quit, update our state safely
            }
        }
    }

} // namespace sandbox::modules