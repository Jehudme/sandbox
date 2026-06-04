#pragma once

#include "sandbox/core/ecs.h"
#include "sandbox/subsystems/runner/irunner.h"
#include <thread>
#include <mutex>
#include <condition_variable>

namespace sandbox::modules {

    class runner : public irunner {
    public:
        // MARK: - Subsystem Lifecycle
        runner(world& ecs);
        ~runner() override;

        // MARK: - Subsystem Implementation
        void start_async(world& ecs) override;
        void run_sync(world& ecs) override;

        void quit() override;
        void pause() override;
        void resume() override;

    private:
        // MARK: - Internal Mechanics
        void internal_tick_loop(world& ecs);

        enum class execution_state {
            Idle,
            Running,
            Paused,
            Quitting
        };

        execution_state m_state{execution_state::Idle};

        std::mutex m_state_mutex;
        std::condition_variable m_state_cv;
        std::thread m_worker_thread;
    };

} // namespace sandbox::modules
