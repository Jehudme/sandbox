#pragma once

#include "sandbox/core/ecs.h"
#include "sandbox/subsystems/runner/irunner.h"
#include <thread>
#include <mutex>
#include <condition_variable>

namespace sandbox::modules {

    class runner : public irunner {
    public:
        runner(world& ecs);
        ~runner() override;

        int32_t start_async(world& ecs) override;
        int32_t run_sync(world& ecs) override;

        int32_t quit() override;
        int32_t pause() override;
        int32_t resume() override;

        void set_property(const char* key, const char* json_value) override;
        int32_t get_property(const char* key, sandbox_payload* out_payload) const override;

    private:
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
        
        float m_fps_limit{60.0f};
    };

} // namespace sandbox::modules
