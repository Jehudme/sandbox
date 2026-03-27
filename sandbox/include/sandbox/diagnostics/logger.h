#pragma once

#include <memory>
#include <string>
#include <string_view>

namespace sandbox
{
    class logger
    {
    public:
        enum class level
        {
            trace,
            debug,
            info,
            warn,
            error,
            critical
        };

        logger(const std::string& logger_name, level log_level = level::info, bool is_async_enabled = false);
        ~logger();

        logger(const logger&) = delete;
        logger& operator=(const logger&) = delete;
        logger(logger&&) noexcept;
        logger& operator=(logger&&) noexcept;

        template<typename... argument_types>
        void log(level lvl, std::string_view format_string, argument_types&&... arguments) const;

        template<typename... argument_types>
        void trace(std::string_view format_string, argument_types&&... arguments) const;

        template<typename... argument_types>
        void debug(std::string_view format_string, argument_types&&... arguments) const;

        template<typename... argument_types>
        void info(std::string_view format_string, argument_types&&... arguments) const;

        template<typename... argument_types>
        void warn(std::string_view format_string, argument_types&&... arguments) const;

        template<typename... argument_types>
        void error(std::string_view format_string, argument_types&&... arguments) const;

        template<typename... argument_types>
        void critical(std::string_view format_string, argument_types&&... arguments) const;

        static std::string level_to_string(level log_level);
        static level string_to_level(std::string_view log_level_string);

    private:
        void _internal_log(level log_level, std::string_view formatted_message) const;

        struct implementation;
        std::unique_ptr<implementation> _implementation;
    };
}

#include "logger.inl"
