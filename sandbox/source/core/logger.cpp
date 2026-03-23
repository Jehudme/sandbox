#include "sandbox/core/logger.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/async.h>

namespace sandbox
{
    struct logger::implementation
    {
        std::shared_ptr<spdlog::logger> spd_logger;
    };

    static spdlog::level::level_enum convert_level(logger::level log_level)
    {
        switch (log_level)
        {
        case logger::level::trace:    return spdlog::level::trace;
        case logger::level::debug:    return spdlog::level::debug;
        case logger::level::info:     return spdlog::level::info;
        case logger::level::warn:     return spdlog::level::warn;
        case logger::level::error:    return spdlog::level::err;
        case logger::level::critical: return spdlog::level::critical;
        default:                      return spdlog::level::info;
        }
    }

    logger::logger(const std::string& logger_name, level log_level, bool is_async_enabled)
        : _implementation(std::make_unique<implementation>())
    {
        if (is_async_enabled)
        {
            spdlog::init_thread_pool(8192, 1);
            _implementation->spd_logger = spdlog::stdout_color_mt<spdlog::async_factory>(logger_name);
        }
        else
        {
            _implementation->spd_logger = spdlog::stdout_color_mt(logger_name);
        }

        _implementation->spd_logger->set_level(convert_level(log_level));
        _implementation->spd_logger->set_pattern("[%T] [%n] [%^%l%$] %v");
    }

    logger::~logger() = default;
    logger::logger(logger&&) noexcept = default;
    logger& logger::operator=(logger&&) noexcept = default;

    void logger::_internal_log(level log_level, std::string_view formatted_message)
    {
        _implementation->spd_logger->log(convert_level(log_level), formatted_message);
    }
}