#pragma once
#include "sandbox/core/plugin.h"

#pragma once

#include <memory>
#include <string>
#include <string_view>

namespace sandbox::plugins
{
    class logger : public plugin
    {
    public:
        enum class log_level { trace, debug, info, warn, error, critical };

        logger() = default;
        ~logger() override = default;

        void initialize(const properties& properties) override;
        void finalize() override;

        template<typename... argument_types>
        void log(log_level lvl, std::string_view format_string, argument_types&&... arguments) const;

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

        static std::string level_to_string(log_level log_level);
        static log_level string_to_level(std::string_view log_level_string);

    private:
        void _internal_log(log_level log_level, std::string_view formatted_message) const;

        struct implementation;
        std::unique_ptr<implementation> _implementation;
    };
}

#include "logger.inl"