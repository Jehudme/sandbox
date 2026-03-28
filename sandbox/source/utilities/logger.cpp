#include "sandbox/utilities/logger.h"
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace sandbox
{
    struct logger::implementation
    {
        std::shared_ptr<spdlog::logger> spd_logger;
    };

    static spdlog::level::level_enum map_level(logger::log_level_t level)
    {
        switch (level)
        {
            case logger::log_level_t::trace:    return spdlog::level::trace;
            case logger::log_level_t::debug:    return spdlog::level::debug;
            case logger::log_level_t::info:     return spdlog::level::info;
            case logger::log_level_t::warn:     return spdlog::level::warn;
            case logger::log_level_t::error:    return spdlog::level::err;
            case logger::log_level_t::critical: return spdlog::level::critical;
            default:                            return spdlog::level::off;
        }
    }

    logger::logger(const std::string& logger_name, log_level_t initial_level, bool enable_async_logging)
        : m_pimpl(std::make_unique<implementation>())
    {
        if (enable_async_logging)   m_pimpl->spd_logger = spdlog::stdout_color_mt<spdlog::async_factory>(logger_name);
        else                        m_pimpl->spd_logger = spdlog::stdout_color_mt(logger_name);

        m_pimpl->spd_logger->set_level(map_level(initial_level));
    }

    logger::~logger() = default;

    logger::logger(logger&& other) noexcept = default;
    logger& logger::operator=(logger&& other) noexcept = default;

    void logger::internal_log(log_level_t level, const std::string& formatted_message) const
    {
        if (m_pimpl && m_pimpl->spd_logger) m_pimpl->spd_logger->log(map_level(level), formatted_message);
    }

    std::string logger::to_string(log_level_t level)
    {
        switch (level)
        {
            case log_level_t::trace:    return "trace";
            case log_level_t::debug:    return "debug";
            case log_level_t::info:     return "info";
            case log_level_t::warn:     return "warn";
            case log_level_t::error:    return "error";
            case log_level_t::critical: return "critical";
            default:                    return "off";
        }
    }

    logger::log_level_t logger::from_string(std::string_view level_string)
    {
        if (level_string == "trace")    return log_level_t::trace;
        if (level_string == "debug")    return log_level_t::debug;
        if (level_string == "info")     return log_level_t::info;
        if (level_string == "warn")     return log_level_t::warn;
        if (level_string == "error")    return log_level_t::error;
        if (level_string == "critical") return log_level_t::critical;
        return log_level_t::off;
    }
}