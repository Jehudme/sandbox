#include "subsystems/logger/logger.h"

#include "sandbox/event_bus/logger_events.h"
#include "sandbox/event_bus/event_bus.h"
#include "sandbox/core/engine.h"
#include "sandbox/utilities/config_helper.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/async.h>
#include <vector>
#include <iostream>
#include <glaze/glaze.hpp>

namespace sandbox::modules {

    logger::logger(flecs::world& ecs) {
        ecs.module<logger>("::Modules::Logger");
        ecs.set<sandbox::logger_service>({this});

        sandbox::properties config;
        auto env_entity = ecs.entity("::Sandbox::Environment");
        if (env_entity.has<engine_environment>()) {
            auto env = env_entity.get<engine_environment>();
            config = env.config;
        }

        std::string logger_name = get_config<std::string>(config, "logger_name", "sandbox_core");
        spdlog::level::level_enum boot_level = get_config<spdlog::level::level_enum>(config, "logger_level", spdlog::level::info);
        m_throw_on_error = get_config<bool>(config, "throw_on_error", false);

        // Fetch the arguments class directly from the ECS world to respect dev_mode override
        // Wait, dev_mode is now in config, not arguments.
        bool dev_mode = get_config<bool>(config, "dev_mode", false);

        // Setup a safe, synchronous "Boot Logger" to capture early startup events
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

        m_logger = std::make_shared<spdlog::logger>(logger_name, console_sink);
        m_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");

        // Evaluate log level (Override config with dev_mode if present)
        if (dev_mode) {
            m_logger->set_level(spdlog::level::trace);
        } else {
            m_logger->set_level(boot_level);
        }

        spdlog::register_logger(m_logger);

        // Using direct spdlog call here to avoid a circular macro dependency
        m_logger->info("[Logger] Boot logger mounted. Awaiting manifest...");
    }

    logger::~logger() {
        if (m_logger) {
            m_logger->info("[Logger] Shutting down.");
            m_logger->flush();
            spdlog::drop(m_logger->name());
        }
    }


    int32_t logger::log(const uint8_t* log_msg_fb, size_t size) {
        if (!m_logger || !log_msg_fb) return -1;

        auto log_msg = flatbuffers::GetRoot<sandbox::schemas::LogMessage>(log_msg_fb);
        if (!log_msg) return -1;

        spdlog::level::level_enum native_spdlog_level = spdlog::level::info;
        bool should_trigger_exception = false;

        switch (log_msg->level()) {
            case sandbox::schemas::LogLevel_Trace: native_spdlog_level = spdlog::level::trace; break;
            case sandbox::schemas::LogLevel_Debug: native_spdlog_level = spdlog::level::debug; break;
            case sandbox::schemas::LogLevel_Info:  native_spdlog_level = spdlog::level::info;  break;
            case sandbox::schemas::LogLevel_Warn:  native_spdlog_level = spdlog::level::warn;  break;

            case sandbox::schemas::LogLevel_Error:
                native_spdlog_level = spdlog::level::err;
                should_trigger_exception = true;
                break;
            case sandbox::schemas::LogLevel_Fatal:
                native_spdlog_level = spdlog::level::critical;
                should_trigger_exception = true;
                break;
        }

        m_logger->log(native_spdlog_level, log_msg->message()->c_str());

        bool final_throw_decision = false;

        if (log_msg->throw_on_error()) {
            final_throw_decision = true;
        } else {
            final_throw_decision = m_throw_on_error && should_trigger_exception;
        }

        if (final_throw_decision) {
            m_logger->flush();
            return -1; // -1 signifies a throw/fatal condition
        }
        return 0;
    }

    void logger::set_property(const char* key, const char* json_value) {
        if (!key || !json_value) return;
        std::string key_str(key);
        if (key_str == "logger_level") {
            spdlog::level::level_enum lvl;
            if (glz::read_json(lvl, json_value) == glz::error_code::none) {
                if (m_logger) {
                    m_logger->set_level(lvl);
                    m_logger->info("[Logger] Log level dynamically updated.");
                }
            } else {
                if (m_logger) m_logger->warn("[Logger] Invalid type for 'logger_level' property.");
            }
        } else if (key_str == "throw_on_error") {
            bool throw_on_err;
            if (glz::read_json(throw_on_err, json_value) == glz::error_code::none) {
                m_throw_on_error = throw_on_err;
            } else {
                if (m_logger) m_logger->warn("[Logger] Invalid type for 'throw_on_error' property.");
            }
        } else {
            if (m_logger) m_logger->warn("[Logger] Unknown property: {}", key);
        }
    }

    int32_t logger::get_property(const char* key, sandbox_payload* out_payload) const {
        if (!key || !out_payload) return -1;
        std::string key_str(key);
        std::string out_json;
        if (key_str == "logger_level") {
            auto lvl = m_logger ? m_logger->level() : spdlog::level::info;
            (void)glz::write_json(lvl, out_json);
        } else if (key_str == "throw_on_error") {
            (void)glz::write_json(m_throw_on_error, out_json);
        } else if (key_str == "logger_name") {
            auto name = m_logger ? m_logger->name() : std::string("");
            (void)glz::write_json(name, out_json);
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
