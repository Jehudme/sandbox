#pragma once

// --- Macro to "Select" (Push) a Logger into the Current Scope ---
// This creates an RAII guard that lasts until the end of the current { } block.
#define SANDBOX_LOGGER_SCOPE(logger_instance) \
sandbox::scoped_logger __scoped_logger_guard_##__LINE__(logger_instance)

// --- Macros to Use the Logger from the Current Scope ---
// These automatically find the active logger on the stack.

#define SANDBOX_LOG_TRACE(format_string, ...) \
sandbox::scoped_logger::get_current_logger().trace(format_string __VA_OPT__(,) __VA_ARGS__)

#define SANDBOX_LOG_DEBUG(format_string, ...) \
sandbox::scoped_logger::get_current_logger().debug(format_string __VA_OPT__(,) __VA_ARGS__)

#define SANDBOX_LOG_INFO(format_string, ...) \
sandbox::scoped_logger::get_current_logger().info(format_string __VA_OPT__(,) __VA_ARGS__)

#define SANDBOX_LOG_WARN(format_string, ...) \
sandbox::scoped_logger::get_current_logger().warn(format_string __VA_OPT__(,) __VA_ARGS__)

#define SANDBOX_LOG_ERROR(format_string, ...) \
sandbox::scoped_logger::get_current_logger().error(format_string __VA_OPT__(,) __VA_ARGS__)

#define SANDBOX_LOG_CRITICAL(format_string, ...) \
sandbox::scoped_logger::get_current_logger().critical(format_string __VA_OPT__(,) __VA_ARGS__)