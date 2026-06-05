#include "subsystems/logger/logger.h"

#include "sandbox/event_bus/logger_events.h"
#include "sandbox/event_bus/event_bus.h"
#include "sandbox/core/engine.h"
#include "sandbox/utilities/config_helper.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/async.h>
#include <vector>
#include <stdexcept>

namespace sandbox::modules {

    logger::logger(flecs::world& ecs) {
        ecs.module<logger>("::Modules::Logger");
        ecs.set<sandbox::logger_service>({this});

        std::unordered_map<std::string, std::any> config;
        auto env_entity = ecs.entity("::Sandbox::Environment");
        if (env_entity.has<engine_environment>()) {
            config = env_entity.get<engine_environment>().config;
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


    /// Emits a log event to the active spdlog sink, handling fatal errors synchronously.
    std::expected<void, std::string> logger::log(const events::log& log_event) {
        if (!m_logger) return {};

        spdlog::level::level_enum native_spdlog_level = spdlog::level::info;
        bool should_trigger_exception = false;

        switch (log_event.log_level) {
            case events::log::level::Trace: native_spdlog_level = spdlog::level::trace; break;
            case events::log::level::Debug: native_spdlog_level = spdlog::level::debug; break;
            case events::log::level::Info:  native_spdlog_level = spdlog::level::info;  break;
            case events::log::level::Warn:  native_spdlog_level = spdlog::level::warn;  break;

            case events::log::level::Error:
                native_spdlog_level = spdlog::level::err;
                should_trigger_exception = true;
                break;
            case events::log::level::Fatal:
                native_spdlog_level = spdlog::level::critical;
                should_trigger_exception = true;
                break;
        }

        m_logger->log(native_spdlog_level, log_event.message);

        bool final_throw_decision = false;

        if (log_event.throw_on_error_override.has_value()) {
            final_throw_decision = log_event.throw_on_error_override.value();
        } else {
            final_throw_decision = m_throw_on_error && should_trigger_exception;
        }

        if (final_throw_decision) {
            m_logger->flush();
            return std::unexpected(log_event.message);
        }
        return {};
    }

    void logger::set_property(const std::string& key, const std::any& value) {
        if (key == "logger_level") {
            if (value.type() == typeid(spdlog::level::level_enum)) {
                auto lvl = std::any_cast<spdlog::level::level_enum>(value);
                if (m_logger) {
                    m_logger->set_level(lvl);
                    m_logger->info("[Logger] Log level dynamically updated.");
                }
            } else {
                if (m_logger) m_logger->warn("[Logger] Invalid type for 'logger_level' property.");
            }
        } else if (key == "throw_on_error") {
            if (value.type() == typeid(bool)) {
                m_throw_on_error = std::any_cast<bool>(value);
            } else {
                if (m_logger) m_logger->warn("[Logger] Invalid type for 'throw_on_error' property.");
            }
        } else {
            if (m_logger) m_logger->warn("[Logger] Unknown property: {}", key);
        }
    }

    std::any logger::get_property(const std::string& key) const {
        if (key == "logger_level") {
            return m_logger ? m_logger->level() : spdlog::level::info;
        } else if (key == "throw_on_error") {
            return m_throw_on_error;
        } else if (key == "logger_name") {
            return m_logger ? m_logger->name() : std::string("");
        }
        return {};
    }

} // namespace sandbox::modules
