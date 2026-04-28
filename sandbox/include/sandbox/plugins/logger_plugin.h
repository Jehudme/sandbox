#pragma once
#include "sandbox/core/plugin.h"

#pragma once

#include <memory>
#include <string>
#include <string_view>

namespace sandbox::plugins
{
    class logger_plugin : public plugin
    {
    public:
        enum class log_level { trace, debug, info, warn, error, critical };

        logger_plugin() = default;
        ~logger_plugin() override = default;

        void initialize() override;
        void finalize() override;

        template<typename... argument_types>
        inline void log(log_level lvl, std::string_view format_string, argument_types&&... arguments) const;

        template<typename... argument_types>
        inline void trace(std::string_view format_string, argument_types&&... arguments) const;

        template<typename... argument_types>
        inline void debug(std::string_view format_string, argument_types&&... arguments) const;

        template<typename... argument_types>
        inline void info(std::string_view format_string, argument_types&&... arguments) const;

        template<typename... argument_types>
        inline void warn(std::string_view format_string, argument_types&&... arguments) const;

        template<typename... argument_types>
        inline void error(std::string_view format_string, argument_types&&... arguments) const;

        template<typename... argument_types>
        inline void critical(std::string_view format_string, argument_types&&... arguments) const;

    private:
        void _internal_log(log_level log_level, std::string_view formatted_message) const;

        struct implementation;
        std::unique_ptr<implementation> _implementation;
    };

    static std::string level_to_string(logger_plugin::log_level log_level);
    static logger_plugin::log_level string_to_level(std::string_view log_level_string);
}

#include "logger_plugin.inl"
