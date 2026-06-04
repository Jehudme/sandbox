#include "modules/runner.h"

#include "sandbox/utilities/events.h"
#include "../../include/sandbox/core/logger.h"
#include "sandbox/utilities/properties.h"

namespace sandbox::modules {

    // MARK: - Subsystem Lifecycle

    runner::runner(world& ecs) {
        ecs.module<runner>("::Modules::Runner");

        // Handshake Listener: Injects the correct execution callback based on the async flag
        sandbox::events::subscribe<events::runner::execution_handshake>(
            ecs,
            [this, &ecs](const events::runner::execution_handshake& event) {
                // If a callback has already been registered by an overriding module, skip
                if (event.callback) return;

                if (event.is_async) {
                    event.callback = [this, &ecs]() { this->start_async(ecs); };
                } else {
                    event.callback = [this, &ecs]() { this->run_sync(ecs); };
                }
            }
        );

        // Interrupt Listener: Manages engine runtime state events
        sandbox::events::subscribe<events::runner::state_change>(
            ecs,
            [this](const events::runner::state_change& event) {
                switch (event.state_request) {
                    case events::runner::state_change::action::Quit:
                        quit();
                        break;
                    case events::runner::state_change::action::Pause:
                        pause();
                        break;
                    case events::runner::state_change::action::Resume:
                        resume();
                        break;
                }
            }
        );
    }

    runner::~runner() {
        quit();

        if (m_worker_thread.joinable()) {
            m_worker_thread.join();
        }
    }

    // MARK: - Subsystem Implementation

    /// Starts the engine execution pipeline asynchronously on a dedicated worker thread.
    void runner::start_async(world& ecs) {
        // Synchronize state transitions to prevent multiple worker threads
        std::lock_guard<std::mutex> lock(m_state_mutex);
        if (m_state != execution_state::Idle) return;

        m_state = execution_state::Running;
        SANDBOX_INFO(ecs, "[Runner] Starting async pipeline thread.");

        m_worker_thread = std::thread(&runner::internal_tick_loop, this, std::ref(ecs));
    }

    /// Executes the tick loop synchronously on the current thread.
    void runner::run_sync(world& ecs) {
        {
            // Mutex scope to safely check and update the run state before entering the tick loop
            std::lock_guard<std::mutex> lock(m_state_mutex);
            if (m_state != execution_state::Idle) return;
            m_state = execution_state::Running;
        }

        SANDBOX_INFO(ecs, "[Runner] Entering main execution loop on the calling thread.");
        internal_tick_loop(ecs);
    }

    /// Signals the tick loop to gracefully terminate.
    void runner::quit() {
        std::lock_guard<std::mutex> lock(m_state_mutex);
        if (m_state == execution_state::Quitting) return;

        m_state = execution_state::Quitting;
        m_state_cv.notify_all();
    }

    /// Pauses the tick loop execution.
    void runner::pause() {
        std::lock_guard<std::mutex> lock(m_state_mutex);
        if (m_state == execution_state::Running) {
            m_state = execution_state::Paused;
        }
    }

    /// Resumes the tick loop execution from a paused state.
    void runner::resume() {
        std::lock_guard<std::mutex> lock(m_state_mutex);
        if (m_state == execution_state::Paused) {
            m_state = execution_state::Running;
            m_state_cv.notify_all();
        }
    }

    /// Core execution loop managed by condition variables for pausing and quitting.
    void runner::internal_tick_loop(world& ecs) {
        ecs.set_target_fps(60);

        while (true) {
            {
                // Wait on condition variable until state is Running or Quitting
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