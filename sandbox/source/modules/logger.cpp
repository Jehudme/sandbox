#include "modules/logger.h"

#include "sandbox/macros/logger.h" // Required to use SANDBOX_INFO
#include "sandbox/utilities/properties.h"
#include "sandbox/utilities/events.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/async.h>
#include <vector>
#include <stdexcept>

namespace sandbox::modules {

    logger::logger(world& ecs) {
        ecs.module<logger>("::Modules::Logger");

        properties manifest = ecs.lookup("::manifest").get<properties>();

        bool is_asynchronous    = manifest.get<bool>({"logger" , "async"})               .value_or(false);
        std::string output_file = manifest.get<std::string>({"logger" , "output_file"})  .value_or("log/sandbox.log");
        std::string log_pattern = manifest.get<std::string>({"logger" , "pattern"})      .value_or("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
        std::string level       = manifest.get<std::string>({"logger" , "level"})        .value_or("info");

        // Ensure the exception override parameter is loaded from the configuration
        m_throw_on_error        = manifest.get<bool>({"logger" , "throw_on_error"})      .value_or(false);

        std::vector<spdlog::sink_ptr> logging_sinks;

        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        logging_sinks.push_back(console_sink);

        if (!output_file.empty()) {
            auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(output_file, true);
            logging_sinks.push_back(file_sink);
        }

        if (is_asynchronous) {
            spdlog::init_thread_pool(8192, 1);
            m_logger = std::make_shared<spdlog::async_logger>(
                "sandbox_core",
                logging_sinks.begin(),
                logging_sinks.end(),
                spdlog::thread_pool(),
                spdlog::async_overflow_policy::block
            );
        } else {
            m_logger = std::make_shared<spdlog::logger>(
                "sandbox_core",
                logging_sinks.begin(),
                logging_sinks.end()
            );
        }

        m_logger->set_pattern(log_pattern);
        m_logger->set_level(spdlog::level::from_str(level));

        spdlog::register_logger(m_logger);

        sandbox::events::subscribe<events::log>(
            ecs,
            [this](const events::log& log_event) {
                this->log(log_event);
            }
        );

        // We use the macro to announce the logger is ready!
        // Because we removed child_of<logger>(), the observer will catch this perfectly.
        SANDBOX_INFO(ecs, "[Logger] Mounted.");
    }

    logger::~logger() {
        if (m_logger) {
            // Direct native call is required here, as the ECS world may be in the middle of teardown/reset
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