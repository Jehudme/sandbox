#pragma once

#include "sandbox/core/ecs.h"
#include "sandbox/events/runner.h"
#include <mutex>
#include <condition_variable>
#include <thread>

namespace sandbox::modules {

    class runner {
    public:
        runner(world& ecs);
        ~runner();

    private:
        void run_sync(world& ecs);
        void start_async(world& ecs);
        void quit();
        void pause();
        void resume();

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
