#pragma once

#include <string>
#include <format>

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
        log(std::format_string<ArgumentTypes...> format_string, level severity, ArgumentTypes&&... arguments);

        std::string message;
        level log_level;
    };

}

#include "logger.inl"
