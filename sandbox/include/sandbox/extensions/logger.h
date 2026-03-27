#pragma once

#include "sandbox/core/extension.h"
#include "sandbox/diagnostics/logger.h"
#include <memory>
#include <string_view>

namespace sandbox::extensions
{
    class logger : public sandbox::extension
    {
    public:
        void initialize(sandbox::engine& app, const sandbox::properties& props) override;
        void finalize(sandbox::engine& app) override;

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

    private:
        std::unique_ptr<sandbox::logger> _logger;
    };
}

#include "logger.inl"
