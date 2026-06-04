#pragma once

#include "sandbox/utilities/events.h"
#include "sandbox/events/logger.h"
#include <optional>

// Internal core macro routing parameters down into the updated structure layout
#define INTERNAL_SANDBOX_LOG_PUBLISH(world_context, severity_enum, throw_override_val, format_literal, ...) \
    sandbox::events::publish( \
        (world_context), \
        sandbox::events::log(__FILE__, __LINE__, sandbox::events::log::level::severity_enum, throw_override_val, format_literal, ##__VA_ARGS__) \
    )

// MARK: - Standard Engine Logging Macros
// Respects global file configuration settings

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

// MARK: - Force-Throw Error Macros
// Guaranteed to bypass config files and throw instantly

#define SANDBOX_WARN_THROW(world, format, ...)  INTERNAL_SANDBOX_LOG_PUBLISH(world, Warn,  true, format, ##__VA_ARGS__)
#define SANDBOX_ERROR_THROW(world, format, ...) INTERNAL_SANDBOX_LOG_PUBLISH(world, Error, true, format, ##__VA_ARGS__)
#define SANDBOX_FATAL_THROW(world, format, ...) INTERNAL_SANDBOX_LOG_PUBLISH(world, Fatal, true, format, ##__VA_ARGS__)