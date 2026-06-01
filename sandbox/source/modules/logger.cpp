#include "modules/logger.h"

#include "sandbox/macros/logger.h"
#include "sandbox/utilities/events.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/async.h>
#include <vector>
#include <stdexcept>

namespace sandbox::modules {

    logger::logger(world& ecs) {
        ecs.module<logger>("::Modules::Logger");

        // 1. Setup a safe, synchronous "Boot Logger" to capture early startup events
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        m_logger = std::make_shared<spdlog::logger>("sandbox_core", console_sink);
        m_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
        m_logger->set_level(spdlog::level::trace); // Capture everything during early boot

        spdlog::register_logger(m_logger);

        // 2. Subscribe to the global logging event bus
        sandbox::events::subscribe<events::log>(
            ecs,
            [this](const events::log& log_event) {
                this->log(log_event);
            }
        );

        // 3. Register an ECS Observer to watch the Manifest entity
        // When the engine finishes reading the VFS and attaches the 'properties' component, this fires automatically!
        flecs::entity manifest_entity = ecs.entity("::manifest");

        ecs.observer<properties>()
           .event(flecs::OnSet)
           .each([this, manifest_entity](flecs::iter& it, size_t index, properties& props) {
               // Only reconfigure if the entity receiving the properties is our global manifest
               if (it.entity(index) == manifest_entity) {
                   this->reconfigure(props);
               }
           });

        SANDBOX_INFO(ecs, "[Logger] Boot logger mounted. Awaiting manifest...");
    }

    logger::~logger() {
        if (m_logger) {
            m_logger->info("[Logger] Shutting down.");
            m_logger->flush();
        }
        spdlog::drop("sandbox_core");
    }

    void logger::reconfigure(const properties& manifest) {
        // 1. Extract settings
        bool is_asynchronous    = manifest.get<bool>({"logger" , "async"})               .value_or(false);
        std::string output_file = manifest.get<std::string>({"logger" , "output_file"})  .value_or("log/sandbox.log");
        std::string log_pattern = manifest.get<std::string>({"logger" , "pattern"})      .value_or("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
        std::string level       = manifest.get<std::string>({"logger" , "level"})        .value_or("info");

        m_throw_on_error        = manifest.get<bool>({"logger" , "throw_on_error"})      .value_or(false);

        // 2. Tear down the old boot logger safely
        spdlog::drop("sandbox_core");

        // 3. Build the new sinks
        std::vector<spdlog::sink_ptr> logging_sinks;
        logging_sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());

        if (!output_file.empty()) {
            logging_sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(output_file, true));
        }

        // 4. Construct the configured target logger
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

        m_logger->info("[Logger] Reconfigured successfully from VFS manifest payload.");
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