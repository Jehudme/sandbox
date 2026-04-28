#include "sandbox/plugins/logger_plugin.h"

#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <vector>

#include "sandbox/core/engine.h"

namespace sandbox::plugins
{
    namespace configuration_keys
    {
        static const std::string category_logger = "logger";
        static const std::string file_path = "file_path";
        static const std::string enable_async_logging = "enable_async";
        static const std::string enable_console_output = "enable_console";
        static const std::string logging_level = "level";
    }

    namespace default_values
    {
        static const std::string log_file_path = "logs/sandbox.log";
        static constexpr bool enable_async_logging = false;
        static constexpr bool enable_console_output = true;
        static const std::string logging_level = "info";
        static const std::string logger_name = "sandbox_logger";
    }

    struct logger_plugin::implementation
    {
        std::shared_ptr<spdlog::logger> spdlog_instance;
    };

    void logger_plugin::initialize()
    {
        _implementation = std::make_unique<implementation>();
        const auto& properties = m_engine.get_properties();

        // 1. Fetch values from properties with defaults
        const std::string log_file_path = properties.get<std::string>({
            configuration_keys::category_logger,
            configuration_keys::file_path
        }).value_or(default_values::log_file_path);

        const bool enable_async_logging = properties.get<bool>({
            configuration_keys::category_logger,
            configuration_keys::enable_async_logging
        }).value_or(default_values::enable_async_logging);

        const bool enable_console_output = properties.get<bool>({
            configuration_keys::category_logger,
            configuration_keys::enable_console_output
        }).value_or(default_values::enable_console_output);

        const std::string logging_level_string = properties.get<std::string>({
            configuration_keys::category_logger,
            configuration_keys::logging_level
        }).value_or(default_values::logging_level);

        // 2. Setup spdlog sinks
        std::vector<spdlog::sink_ptr> logger_sinks;

        if (enable_console_output)
        {
            logger_sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
        }

        if (!log_file_path.empty())
        {
            // true parameter means truncate the log file on start
            logger_sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file_path, true));
        }

        // 3. Create the logger (Async vs Sync)
        if (enable_async_logging)
        {
            spdlog::init_thread_pool(8192, 1);
            _implementation->spdlog_instance = std::make_shared<spdlog::async_logger>(
                default_values::logger_name,
                logger_sinks.begin(),
                logger_sinks.end(),
                spdlog::thread_pool(),
                spdlog::async_overflow_policy::block
            );
        }
        else
        {
            _implementation->spdlog_instance = std::make_shared<spdlog::logger>(
                default_values::logger_name,
                logger_sinks.begin(),
                logger_sinks.end()
            );
        }

        // 4. Set logging level and register
        const log_level configured_plugin_level = string_to_level(logging_level_string);
        spdlog::level::level_enum target_spdlog_level = spdlog::level::info;

        switch (configured_plugin_level)
        {
            case log_level::trace:    target_spdlog_level = spdlog::level::trace; break;
            case log_level::debug:    target_spdlog_level = spdlog::level::debug; break;
            case log_level::info:     target_spdlog_level = spdlog::level::info; break;
            case log_level::warn:     target_spdlog_level = spdlog::level::warn; break;
            case log_level::error:    target_spdlog_level = spdlog::level::err; break;
            case log_level::critical: target_spdlog_level = spdlog::level::critical; break;
        }

        _implementation->spdlog_instance->set_level(target_spdlog_level);
        spdlog::register_logger(_implementation->spdlog_instance);
    }

    void logger_plugin::finalize()
    {
        if (_implementation && _implementation->spdlog_instance)
        {
            _implementation->spdlog_instance->flush();
            spdlog::drop(_implementation->spdlog_instance->name());
        }
        _implementation.reset();
    }

    void logger_plugin::_internal_log(log_level log_level, std::string_view formatted_message) const
    {
        if (!_implementation || !_implementation->spdlog_instance) return;

        spdlog::level::level_enum target_spdlog_level = spdlog::level::info;
        switch (log_level)
        {
            case log_level::trace:    target_spdlog_level = spdlog::level::trace; break;
            case log_level::debug:    target_spdlog_level = spdlog::level::debug; break;
            case log_level::info:     target_spdlog_level = spdlog::level::info; break;
            case log_level::warn:     target_spdlog_level = spdlog::level::warn; break;
            case log_level::error:    target_spdlog_level = spdlog::level::err; break;
            case log_level::critical: target_spdlog_level = spdlog::level::critical; break;
        }

        _implementation->spdlog_instance->log(target_spdlog_level, formatted_message);
    }

    std::string level_to_string(logger_plugin::log_level log_level)
    {
        switch (log_level)
        {
            case logger_plugin::log_level::trace:    return "trace";
            case logger_plugin::log_level::debug:    return "debug";
            case logger_plugin::log_level::info:     return "info";
            case logger_plugin::log_level::warn:     return "warn";
            case logger_plugin::log_level::error:    return "error";
            case logger_plugin::log_level::critical: return "critical";
            default:                                 return "info";
        }
    }

    logger_plugin::log_level string_to_level(std::string_view log_level_string)
    {
        if (log_level_string == "trace")    return logger_plugin::log_level::trace;
        if (log_level_string == "debug")    return logger_plugin::log_level::debug;
        if (log_level_string == "info")     return logger_plugin::log_level::info;
        if (log_level_string == "warn")     return logger_plugin::log_level::warn;
        if (log_level_string == "error")    return logger_plugin::log_level::error;
        if (log_level_string == "critical") return logger_plugin::log_level::critical;

        return logger_plugin::log_level::info;
    }
}
