#include "logger.h"

#include "sandbox/utilities/properties.h"
#include "sandbox/utilities/events.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/async.h>
#include <vector>

#include "spdlog/sinks/basic_file_sink.h"

namespace sandbox::modules {

    logger::logger(world& ecs) {
        // 1. Register this class structure as an official Flecs module context name scope
        ecs.module<logger>("::Modules::Logger");

        // 2. Extract properties configuration component from the global manifest entity
        properties manifest = ecs.lookup("::manifest").get<properties>();

        // 3. Retrieve spdlog construction parameters with safe fallback defaults
        bool is_asynchronous       = manifest.get<bool>({"logger" , "async"})               .value_or(false);
        std::string output_file    = manifest.get<std::string>({"logger" , "output_file"})  .value_or("log/sandbox.log");
        std::string log_pattern    = manifest.get<std::string>({"logger" , "pattern"})      .value_or("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
        std::string level          = manifest.get<std::string>({"logger" , "level"})        .value_or("info");

        // 4. Set up individual sinks (Console color-coded terminal output + Persistent disk file)
        std::vector<spdlog::sink_ptr> logging_sinks;

        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        logging_sinks.push_back(console_sink);

        if (!output_file.empty()) {
            auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(output_file, true);
            logging_sinks.push_back(file_sink);
        }

        // 5. Build either an Async thread-pool variant or a direct Sync logging implementation
        if (is_asynchronous) {
            // Allocate backing thread ring-buffer worker context cleanly if not already initialized
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

        // 6. Apply pattern layouts and severity filtering levels to our spdlog controller
        m_logger->set_pattern(log_pattern);
        m_logger->set_level(spdlog::level::from_str(level));

        // Register with global spdlog registry handle so macros can find it if needed
        spdlog::register_logger(m_logger);

        // 7. Subscribe to the custom type-safe event channel we designed earlier
        flecs::entity subscriber_observer = sandbox::events::subscribe<events::log>(
            ecs,
            [this](const events::log& log_event) {
                this->log(log_event);
            }
        );

        subscriber_observer.child_of<logger>();

        m_logger->info("[LoggerModule] Service fully mounted and intercepting global channel telemetry.");
        sandbox::events::publish(ecs, events::log("Sandbox Engine initialized successfully.", events::log::level::Info));
    }

    logger::~logger() {
        if (m_logger) {
            m_logger->info("[LoggerModule] Flushing stream buffers and shutting down cleanly.");
            m_logger->flush();
        }
        spdlog::drop("sandbox_core");
    }

    void logger::log(const events::log& log_event) {
        if (!m_logger) return;

        // Map your engine's custom event enum layer cleanly into native spdlog severity levels
        spdlog::level::level_enum native_spdlog_level = spdlog::level::info;

        switch (log_event.log_level) {
            case events::log::level::Trace: native_spdlog_level = spdlog::level::trace; break;
            case events::log::level::Debug: native_spdlog_level = spdlog::level::debug; break;
            case events::log::level::Info:  native_spdlog_level = spdlog::level::info;  break;
            case events::log::level::Warn:  native_spdlog_level = spdlog::level::warn;  break;
            case events::log::level::Error: native_spdlog_level = spdlog::level::err;   break;
            case events::log::level::Fatal: native_spdlog_level = spdlog::level::critical; break;
        }

        // Output raw pre-formatted string data straight into the configured sinks
        m_logger->log(native_spdlog_level, log_event.message);
    }

} // namespace sandbox::modules