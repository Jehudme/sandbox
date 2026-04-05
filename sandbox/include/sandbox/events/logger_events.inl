#pragma once

namespace sandbox::events
{
    template <typename... argument_types>
    log_event log_event::create_log_event(plugins::logger_plugin::log_level level, std::string_view format_string,auto&&... arguments) {
        return log_event(level, std::format(format_string, std::forward<argument_types>(arguments)...));
    }

    template <typename... argument_types>
    log_event log_event::create_log_trace_event(std::string_view format_string, argument_types&&... arguments) {
        return create_log_event<argument_types>(plugins::logger_plugin::log_level::trace, format_string, std::forward<argument_types>(arguments)...);
    }

    template <typename... argument_types>
    log_event log_event::create_log_debug_event(std::string_view format_string, argument_types&&... arguments) {
        return create_log_event<argument_types>(plugins::logger_plugin::log_level::debug, format_string, std::forward<argument_types>(arguments)...);
    }

    template <typename... argument_types>
    log_event log_event::create_log_info_event(std::string_view format_string, argument_types&&... arguments) {
        return create_log_event<argument_types>(plugins::logger_plugin::log_level::info, format_string, std::forward<argument_types>(arguments)...);
    }

    template <typename... argument_types>
    log_event log_event::create_log_warn_event(std::string_view format_string, argument_types&&... arguments) {
        return create_log_event<argument_types>(plugins::logger_plugin::log_level::warn, format_string, std::forward<argument_types>(arguments)...);
    }

    template <typename... argument_types>
    log_event log_event::create_log_error_event(std::string_view format_string, argument_types&&... arguments) {
        return create_log_event<argument_types>(plugins::logger_plugin::log_level::error, format_string, std::forward<argument_types>(arguments)...);
    }

    template <typename... argument_types>
    log_event log_event::create_log_critical_event(std::string_view format_string, argument_types&&... arguments) {
        return create_log_event<argument_types>(plugins::logger_plugin::log_level::critical, format_string, std::forward<argument_types>(arguments)...);
    }
}
