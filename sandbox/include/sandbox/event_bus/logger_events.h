#pragma once

#include <string>
#include <format>
#include <optional>
#include "sandbox/subsystems/logger/ilogger.h"
#include "sandbox/generated/schemas/common_generated.h"
#include <flatbuffers/flatbuffers.h>

#define INTERNAL_SANDBOX_LOG_PUBLISH(world_context, severity_enum, throw_override_val, format_literal, ...) \
    do { \
        if ((world_context).has<sandbox::logger_service>()) { \
            std::string msg; \
            try { \
                msg = std::format(format_literal, ##__VA_ARGS__); \
            } catch (const std::exception& e) { \
                msg = std::string("FORMAT ERROR: ") + e.what(); \
            } \
            flatbuffers::FlatBufferBuilder builder; \
            auto fmsg = builder.CreateString(msg); \
            auto ffile = builder.CreateString(__FILE__); \
            sandbox::schemas::LogMessageBuilder lmb(builder); \
            lmb.add_level(sandbox::schemas::LogLevel_##severity_enum); \
            lmb.add_message(fmsg); \
            lmb.add_source_file(ffile); \
            lmb.add_source_line(__LINE__); \
            std::optional<bool> override_val = throw_override_val; \
            lmb.add_throw_on_error(override_val.value_or(false)); \
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

#define SANDBOX_WARN_THROW(world, format, ...)  INTERNAL_SANDBOX_LOG_PUBLISH(world, Warn,  std::optional<bool>{true}, format, ##__VA_ARGS__)
#define SANDBOX_ERROR_THROW(world, format, ...) INTERNAL_SANDBOX_LOG_PUBLISH(world, Error, std::optional<bool>{true}, format, ##__VA_ARGS__)
#define SANDBOX_FATAL_THROW(world, format, ...) INTERNAL_SANDBOX_LOG_PUBLISH(world, Fatal, std::optional<bool>{true}, format, ##__VA_ARGS__)
