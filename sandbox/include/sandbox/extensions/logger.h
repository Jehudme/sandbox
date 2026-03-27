#pragma once

#include "sandbox/core/extension.h"
#include "sandbox/diagnostics/logger.h"
#include <flecs.h>
#include <memory>
#include <string_view>

namespace sandbox::extensions
{
    class logger : public sandbox::extension
    {
    public:
        void initialize(const sandbox::properties& props) override;
        void finalize() override;

        template<typename... argument_types>
        void trace(std::string_view format_string, argument_types&&... arguments);

        template<typename... argument_types>
        void debug(std::string_view format_string, argument_types&&... arguments);

        template<typename... argument_types>
        void info(std::string_view format_string, argument_types&&... arguments);

        template<typename... argument_types>
        void warn(std::string_view format_string, argument_types&&... arguments);

        template<typename... argument_types>
        void error(std::string_view format_string, argument_types&&... arguments);

        template<typename... argument_types>
        void critical(std::string_view format_string, argument_types&&... arguments);

        void enable() const;
        void disable() const;

        bool enabled() const;

    private:
        // Cached entity and raw logger pointer to avoid repeated world lookups on every log call.
        // Both are set during initialize() and cleared during finalize().
        mutable flecs::entity _logger_entity;
        sandbox::logger* _cached_logger_ptr = nullptr;
    };
}

#include "logger.inl"
