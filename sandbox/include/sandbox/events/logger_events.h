#pragma once
#include "sandbox/plugins/logger_plugin.h"

namespace sandbox::events {
    struct log_event
    {
        log_event(plugins::logger_plugin::log_level level, std::string formatted_message);

        template<typename... argument_types>
        static log_event create_log_event(plugins::logger_plugin::log_level level, std::string_view format_string, auto&&... arguments);

        template<typename... argument_types>
        static log_event create_log_trace_event(std::string_view format_string, argument_types&&... arguments);

        template<typename... argument_types>
        static log_event create_log_debug_event(std::string_view format_string, argument_types&&... arguments);

        template<typename... argument_types>
        static log_event create_log_info_event(std::string_view format_string, argument_types&&... arguments);

        template<typename... argument_types>
        static log_event create_log_warn_event(std::string_view format_string, argument_types&&... arguments);

        template<typename... argument_types>
        static log_event create_log_error_event(std::string_view format_string, argument_types&&... arguments);

        template<typename... argument_types>
        static log_event create_log_critical_event(std::string_view format_string, argument_types&&... arguments);

        const std::string_view message;
        const plugins::logger_plugin::log_level level;
    };

}

#include "logger_events.inl"
