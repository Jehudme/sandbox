#pragma once

// Private header — included only from engine.cpp and logger.cpp.
// Defines sandbox::extensions::logger: an extension wrapper around
// sandbox::logger that participates in the engine extension system.

#include "sandbox/diagnostics/logger.h"
#include "sandbox/core/extension.h"
#include <spdlog/spdlog.h>

namespace sandbox::extensions
{
    class logger : public sandbox::extension
    {
    public:
        logger() = default;
        ~logger() override = default;

        void initialize(const sandbox::properties& props) override
        {
            const std::string name = props
                .get<std::string>({"name"})
                .value_or("sandbox");

            const std::string level_str = props
                .get<std::string>({"level"})
                .value_or("INFO");

            const bool async = props
                .get<bool>({"async"})
                .value_or(false);

            // Drop any pre-existing spdlog logger registered under this name
            // (e.g. the scoped_logger fallback) before creating the real one.
            spdlog::drop(name);

            _logger = std::make_unique<sandbox::logger>(
                name,
                sandbox::logger::string_to_level(level_str),
                async);
        }

        void finalize() override
        {
            _logger.reset();
        }

        sandbox::logger* get() const { return _logger.get(); }

        template<typename... argument_types>
        void trace(std::string_view fmt, argument_types&&... args) const
        {
            if (_logger) _logger->trace(fmt, std::forward<argument_types>(args)...);
        }

        template<typename... argument_types>
        void debug(std::string_view fmt, argument_types&&... args) const
        {
            if (_logger) _logger->debug(fmt, std::forward<argument_types>(args)...);
        }

        template<typename... argument_types>
        void info(std::string_view fmt, argument_types&&... args) const
        {
            if (_logger) _logger->info(fmt, std::forward<argument_types>(args)...);
        }

        template<typename... argument_types>
        void warn(std::string_view fmt, argument_types&&... args) const
        {
            if (_logger) _logger->warn(fmt, std::forward<argument_types>(args)...);
        }

        template<typename... argument_types>
        void error(std::string_view fmt, argument_types&&... args) const
        {
            if (_logger) _logger->error(fmt, std::forward<argument_types>(args)...);
        }

        template<typename... argument_types>
        void critical(std::string_view fmt, argument_types&&... args) const
        {
            if (_logger) _logger->critical(fmt, std::forward<argument_types>(args)...);
        }

    private:
        std::unique_ptr<sandbox::logger> _logger;
    };
}
