#pragma once
#include <format>

#include "sandbox/core/extension.h"
#include "sandbox/utilities/logger.h"
#include "spdlog/sinks/win_eventlog_sink.h"

namespace sandbox::extensions
{
    class logger : public extension
    {
    public:
        enum level { trace, debug, info, warn, error, critical };

        logger() = default;
        ~logger() override = default;

        void initialize(const properties& properties) override;
        void finalize() override;

        template<typename... args>
        void log(level level, std::format_string<args...> format, args&&... arguments) const;

        template<typename... args>
        void trace(std::format_string<args...> format, args&&... arguments) const;

        template<typename... args>
        void debug(std::format_string<args...> format, args&&... arguments) const;

        template<typename... args>
        void info(std::format_string<args...> format, args&&... arguments) const;

        template<typename... args>
        void warn(std::format_string<args...> format, args&&... arguments) const;

        template<typename... args>
        void critical(std::format_string<args...> format, args&&... arguments) const;

    private:
        void implementation_log(logger::level level, const std::string& formatted_message) const;
    };
}

#include "logger.inl"
