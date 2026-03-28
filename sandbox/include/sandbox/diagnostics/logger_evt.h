#pragma once

#include <string>
#include <string_view>

namespace sandbox::diagnostics
{
    struct logger_trace_evt
    {
        std::string message;

        static logger_trace_evt create(std::string_view message)
        {
            return {std::string(message)};
        }
    };

    struct logger_debug_evt
    {
        std::string message;

        static logger_debug_evt create(std::string_view message)
        {
            return {std::string(message)};
        }
    };

    struct logger_info_evt
    {
        std::string message;

        static logger_info_evt create(std::string_view message)
        {
            return {std::string(message)};
        }
    };

    struct logger_warn_evt
    {
        std::string message;

        static logger_warn_evt create(std::string_view message)
        {
            return {std::string(message)};
        }
    };

    struct logger_error_evt
    {
        std::string message;

        static logger_error_evt create(std::string_view message)
        {
            return {std::string(message)};
        }
    };

    struct logger_critical_evt
    {
        std::string message;

        static logger_critical_evt create(std::string_view message)
        {
            return {std::string(message)};
        }
    };
}
