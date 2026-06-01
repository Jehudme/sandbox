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

        // Added throw_override parameter to constructor signature
        template<typename... ArgumentTypes>
        log(const char* file, int line, level severity, std::optional<bool> throw_override, std::format_string<ArgumentTypes...> format_string, ArgumentTypes&&... arguments);

        std::string message;
        level log_level;
        const char* source_file;
        int source_line;
        std::optional<bool> throw_on_error_override; // Captures if this specific log should force or prevent an exception throw
    };

} // namespace sandbox::events

#include "logger.inl"