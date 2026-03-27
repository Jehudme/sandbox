#pragma once
#include "sandbox/core/engine.h"

namespace sandbox::extensions
{
    template<typename... argument_types>
    void logger::trace(std::string_view format_string, argument_types&&... arguments)
    {
        if (!_cached_logger_ptr || !enabled()) return;
        _cached_logger_ptr->trace(format_string, std::forward<argument_types>(arguments)...);
    }

    template<typename... argument_types>
    void logger::debug(std::string_view format_string, argument_types&&... arguments)
    {
        if (!_cached_logger_ptr || !enabled()) return;
        _cached_logger_ptr->debug(format_string, std::forward<argument_types>(arguments)...);
    }

    template<typename... argument_types>
    void logger::info(std::string_view format_string, argument_types&&... arguments)
    {
        if (!_cached_logger_ptr || !enabled()) return;
        _cached_logger_ptr->info(format_string, std::forward<argument_types>(arguments)...);
    }

    template<typename... argument_types>
    void logger::warn(std::string_view format_string, argument_types&&... arguments)
    {
        if (!_cached_logger_ptr || !enabled()) return;
        _cached_logger_ptr->warn(format_string, std::forward<argument_types>(arguments)...);
    }

    template<typename... argument_types>
    void logger::error(std::string_view format_string, argument_types&&... arguments)
    {
        if (!_cached_logger_ptr || !enabled()) return;
        _cached_logger_ptr->error(format_string, std::forward<argument_types>(arguments)...);
    }

    template<typename... argument_types>
    void logger::critical(std::string_view format_string, argument_types&&... arguments)
    {
        if (!_cached_logger_ptr || !enabled()) return;
        _cached_logger_ptr->critical(format_string, std::forward<argument_types>(arguments)...);
    }
}

