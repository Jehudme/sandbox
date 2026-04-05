#pragma once

#include <format>
#include <string_view>
#include <utility>

namespace sandbox::events
{
    template <typename... Args>
    log_event create_log_event(plugins::logger_plugin::log_level level, std::string_view format_string, Args&&... args) {
        return log_event(level, std::vformat(format_string, std::make_format_args(std::forward<Args>(args)...)));
    }

    template <typename... Args>
    log_event create_log_trace_event(std::string_view format_string, Args&&... args) {
        return create_log_event<Args...>(plugins::logger_plugin::log_level::trace, format_string, std::forward<Args>(args)...);
    }

    template <typename... Args>
    log_event create_log_debug_event(std::string_view format_string, Args&&... args) {
        return create_log_event<Args...>(plugins::logger_plugin::log_level::debug, format_string, std::forward<Args>(args)...);
    }

    template <typename... Args>
    log_event create_log_info_event(std::string_view format_string, Args&&... args) {
        return create_log_event<Args...>(plugins::logger_plugin::log_level::info, format_string, std::forward<Args>(args)...);
    }

    template <typename... Args>
    log_event create_log_warn_event(std::string_view format_string, Args&&... args) {
        return create_log_event<Args...>(plugins::logger_plugin::log_level::warn, format_string, std::forward<Args>(args)...);
    }

    template <typename... Args>
    log_event create_log_error_event(std::string_view format_string, Args&&... args) {
        return create_log_event<Args...>(plugins::logger_plugin::log_level::error, format_string, std::forward<Args>(args)...);
    }

    template <typename... Args>
    log_event create_log_critical_event(std::string_view format_string, Args&&... args) {
        return create_log_event<Args...>(plugins::logger_plugin::log_level::critical, format_string, std::forward<Args>(args)...);
    }
}