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

#include "sandbox/subsystems/logger/ilogger.h"

#include "sandbox/generated/schemas/common_generated.h"
#include <flatbuffers/flatbuffers.h>

#define INTERNAL_SANDBOX_LOG_PUBLISH(world_context, severity_enum, throw_override_val, format_literal, ...) \
    do { \
        if ((world_context).has<sandbox::logger_service>()) { \
            auto event_obj = sandbox::events::log(__FILE__, __LINE__, sandbox::events::log::level::severity_enum, throw_override_val, format_literal, ##__VA_ARGS__); \
            flatbuffers::FlatBufferBuilder builder; \
            auto msg = builder.CreateString(event_obj.message); \
            auto file = builder.CreateString(event_obj.source_file); \
            sandbox::schemas::LogMessageBuilder lmb(builder); \
            lmb.add_level(static_cast<sandbox::schemas::LogLevel>(event_obj.log_level)); \
            lmb.add_message(msg); \
            lmb.add_source_file(file); \
            lmb.add_source_line(event_obj.source_line); \
            lmb.add_throw_on_error(event_obj.throw_on_error_override.value_or(false)); \
            builder.Finish(lmb.Finish()); \
            (world_context).get<sandbox::logger_service>().api->log(builder.GetBufferPointer(), builder.GetSize()); \
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
