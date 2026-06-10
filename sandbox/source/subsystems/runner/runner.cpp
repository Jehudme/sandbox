#include "subsystems/runner/runner.h"
#include "sandbox/core/environment.h"

#include "sandbox/event_bus/event_bus.h"
#include "sandbox/subsystems/logger/ilogger.h"
#include "sandbox/utilities/properties.h"
#include "sandbox/core/engine.h"
#include "sandbox/utilities/config_helper.h"
#include <thread>
#include <chrono>
#include <glaze/glaze.hpp>

namespace sandbox::modules {

    runner::runner(world& ecs) {
        ecs.module<runner>("::Modules::Runner");
        ecs.set<sandbox::runner_service>({this});

        sandbox::properties config;
        auto env_entity = ecs.entity("::Sandbox::Environment");
        if (env_entity.has<engine_environment>()) {
            auto env = env_entity.get<engine_environment>();
            config = env.config;
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

    int32_t runner::start_async(world& ecs) {
        std::lock_guard<std::mutex> lock(m_state_mutex);
        if (m_state != execution_state::Idle) return -1;

        m_state = execution_state::Running;
        SANDBOX_INFO(ecs, "[Runner] Starting async pipeline thread.");

        m_worker_thread = std::thread(&runner::internal_tick_loop, this, std::ref(ecs));
        return 0;
    }

    int32_t runner::run_sync(world& ecs) {
        {
            std::lock_guard<std::mutex> lock(m_state_mutex);
            if (m_state != execution_state::Idle) return -1;
            m_state = execution_state::Running;
        }

        SANDBOX_INFO(ecs, "[Runner] Entering main execution loop on the calling thread.");
        internal_tick_loop(ecs);
        return 0;
    }

    int32_t runner::quit() {
        std::lock_guard<std::mutex> lock(m_state_mutex);
        if (m_state == execution_state::Quitting) return 0;

        m_state = execution_state::Quitting;
        m_state_cv.notify_all();
        return 0;
    }

    int32_t runner::pause() {
        std::lock_guard<std::mutex> lock(m_state_mutex);
        if (m_state == execution_state::Running) {
            m_state = execution_state::Paused;
        }
        return 0;
    }

    int32_t runner::resume() {
        std::lock_guard<std::mutex> lock(m_state_mutex);
        if (m_state == execution_state::Paused) {
            m_state = execution_state::Running;
            m_state_cv.notify_all();
        }
        return 0;
    }

    // MARK: - Internal Mechanics

    /// Main execution loop managing frame progression and termination signals.
    void runner::internal_tick_loop(world& ecs) {
        ecs.set_target_fps(m_fps_limit);

        try {
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
        } catch (const std::exception& e) {
            SANDBOX_FATAL(ecs, "[Runner] Fatal exception in tick loop: {}", e.what());
            quit();
        }
    }

    void runner::set_property(const char* key, const char* json_value) {
        if (!key || !json_value) return;
        std::string key_str(key);
        if (key_str == "fps_limit") {
            float fps;
            if (glz::read_json(fps, json_value) == glz::error_code::none) {
                m_fps_limit = fps;
            }
        }
    }

    int32_t runner::get_property(const char* key, sandbox_payload* out_payload) const {
        if (!key || !out_payload) return -1;
        std::string key_str(key);
        std::string out_json;
        if (key_str == "fps_limit") {
            (void)glz::write_json(m_fps_limit, out_json);
        } else {
            return -1;
        }
        
        uint8_t* ptr = static_cast<uint8_t*>(std::malloc(out_json.size() + 1));
        std::memcpy(ptr, out_json.c_str(), out_json.size() + 1);
        out_payload->bytes = ptr;
        out_payload->size = out_json.size();
        out_payload->free_func = [](void* p) { std::free(p); };
        return 0;
    }

} // namespace sandbox::modules
