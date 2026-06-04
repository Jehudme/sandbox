#pragma once

#include <string>
#include <format>
#include <optional>

namespace sandbox::events {

    struct log {
        enum class level {
            Trace,
            Debug,
            Info,
            Warn,
            Error,
            Fatal
        };

        template<typename... ArgumentTypes>
        log(const char* file, int line, level severity, std::optional<bool> throw_override,
            std::format_string<ArgumentTypes...> format_string, ArgumentTypes&&... arguments);

        std::string message;
        level log_level;
        const char* source_file;
        int source_line;
        std::optional<bool> throw_on_error_override;
    };

} // namespace sandbox::events

#include "detail/logger_events.inl"

// MARK: - Log Macros

#include "sandbox/subsystems/logger/ilogger.h"

#define INTERNAL_SANDBOX_LOG_PUBLISH(world_context, severity_enum, throw_override_val, format_literal, ...) \
    do { \
        if ((world_context).has<sandbox::logger_service>()) { \
            (world_context).get<sandbox::logger_service>().api->log(sandbox::events::log(__FILE__, __LINE__, sandbox::events::log::level::severity_enum, throw_override_val, format_literal, ##__VA_ARGS__)); \
        } \
    } while(0)

#ifndef NDEBUG
    #define SANDBOX_TRACE(world, format, ...) INTERNAL_SANDBOX_LOG_PUBLISH(world, Trace, std::nullopt, format, ##__VA_ARGS__)
    #define SANDBOX_DEBUG(world, format, ...) INTERNAL_SANDBOX_LOG_PUBLISH(world, Debug, std::nullopt, format, ##__VA_ARGS__)
#else
    #define SANDBOX_TRACE(world, format, ...) (void)0
    #define SANDBOX_DEBUG(world, format, ...) (void)0
#endif

#define SANDBOX_INFO(world, format, ...)  INTERNAL_SANDBOX_LOG_PUBLISH(world, Info,  std::nullopt, format, ##__VA_ARGS__)
#define SANDBOX_WARN(world, format, ...)  INTERNAL_SANDBOX_LOG_PUBLISH(world, Warn,  std::nullopt, format, ##__VA_ARGS__)
#define SANDBOX_ERROR(world, format, ...) INTERNAL_SANDBOX_LOG_PUBLISH(world, Error, std::nullopt, format, ##__VA_ARGS__)
#define SANDBOX_FATAL(world, format, ...) INTERNAL_SANDBOX_LOG_PUBLISH(world, Fatal, std::nullopt, format, ##__VA_ARGS__)

#define SANDBOX_WARN_THROW(world, format, ...)  INTERNAL_SANDBOX_LOG_PUBLISH(world, Warn,  true, format, ##__VA_ARGS__)
#define SANDBOX_ERROR_THROW(world, format, ...) INTERNAL_SANDBOX_LOG_PUBLISH(world, Error, true, format, ##__VA_ARGS__)
#define SANDBOX_FATAL_THROW(world, format, ...) INTERNAL_SANDBOX_LOG_PUBLISH(world, Fatal, true, format, ##__VA_ARGS__)
