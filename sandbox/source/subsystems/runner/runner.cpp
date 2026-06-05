#include "subsystems/runner/runner.h"

#include "sandbox/event_bus/event_bus.h"
#include "sandbox/event_bus/logger_events.h"
#include "sandbox/utilities/properties.h"
#include "sandbox/core/engine.h"
#include "sandbox/utilities/config_helper.h"

namespace sandbox::modules {

    runner::runner(world& ecs) {
        ecs.module<runner>("::Modules::Runner");
        ecs.set<sandbox::runner_service>({this});

        std::unordered_map<std::string, std::any> config;
        auto env_entity = ecs.entity("::Sandbox::Environment");
        if (env_entity.has<engine_environment>()) {
            config = env_entity.get<engine_environment>().config;
        }

        int default_fps = 60;
        int fps = get_config<int>(config, "fps_limit", default_fps);
        m_fps_limit = static_cast<float>(fps);
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

    // MARK: - Internal Mechanics

    /// Main execution loop managing frame progression and termination signals.
    void runner::internal_tick_loop(world& ecs) {
        ecs.set_target_fps(m_fps_limit);

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

    void runner::set_property(const std::string& key, const std::any& value) {
        if (key == "fps_limit") {
            if (value.type() == typeid(int)) {
                m_fps_limit = static_cast<float>(std::any_cast<int>(value));
                // We'd ideally call ecs.set_target_fps here, but we don't store ecs.
                // However, internal_tick_loop calls ecs.set_target_fps initially. 
                // To dynamically update, we need a reference to ecs or wait until next progress.
            } else if (value.type() == typeid(float)) {
                m_fps_limit = std::any_cast<float>(value);
            } else {
                // Cannot easily log without ecs ref, but could throw or ignore
            }
        }
    }

    std::any runner::get_property(const std::string& key) const {
        if (key == "fps_limit") {
            return m_fps_limit;
        }
        return {};
    }

} // namespace sandbox::modules
