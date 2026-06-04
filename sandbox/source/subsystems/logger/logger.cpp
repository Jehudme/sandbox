#include "subsystems/logger/logger.h"

#include "sandbox/event_bus/logger_events.h"
#include "sandbox/event_bus/event_bus.h"
#include "sandbox/core/engine.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/async.h>
#include <vector>
#include <stdexcept>

namespace sandbox::modules {

    logger::logger(flecs::world& ecs, const logger_config& config)
        : m_throw_on_error(config.throw_on_error)
    {
        ecs.module<logger>("::Modules::Logger");

        // Fetch the arguments class directly from the ECS world
        auto args = ecs.entity("::Sandbox::Arguments").get<engine::arguments>();

        // 1. Setup a safe, synchronous "Boot Logger" to capture early startup events
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

        m_logger = std::make_shared<spdlog::logger>(config.logger_name, console_sink);
        m_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");

        // 2. Evaluate log level (Override config with dev_mode if present)
        if (args.dev_mode) {
            m_logger->set_level(spdlog::level::trace);
        } else {
            m_logger->set_level(config.boot_level);
        }

        spdlog::register_logger(m_logger);

        // 3. Subscribe to the global logging event bus
        sandbox::events::subscribe<events::log>(
            ecs,
            [this](const events::log& log_event) {
                this->log(log_event);
            }
        );

        // Using direct spdlog call here to avoid a circular macro dependency
        m_logger->info("[Logger] Boot logger mounted. Awaiting manifest...");
    }

    logger::~logger() {
        if (m_logger) {
            m_logger->info("[Logger] Shutting down.");
            m_logger->flush();
        }
        spdlog::drop("sandbox_core");
    }

    void logger::log(const events::log& log_event) {
        if (!m_logger) return;

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
            throw std::runtime_error(log_event.message);
        }
    }

} // namespace sandbox::modules
