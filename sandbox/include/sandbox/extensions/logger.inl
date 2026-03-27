#pragma once

namespace sandbox::extensions
{
    template<typename... argument_types>
    void logger::trace(std::string_view format_string, argument_types&&... arguments)
    {
        if (!_logger) return;
        _logger->trace(format_string, std::forward<argument_types>(arguments)...);
    }

    template<typename... argument_types>
    void logger::debug(std::string_view format_string, argument_types&&... arguments)
    {
        if (!_logger) return;
        _logger->debug(format_string, std::forward<argument_types>(arguments)...);
    }

    template<typename... argument_types>
    void logger::info(std::string_view format_string, argument_types&&... arguments)
    {
        if (!_logger) return;
        _logger->info(format_string, std::forward<argument_types>(arguments)...);
    }

    template<typename... argument_types>
    void logger::warn(std::string_view format_string, argument_types&&... arguments)
    {
        if (!_logger) return;
        _logger->warn(format_string, std::forward<argument_types>(arguments)...);
    }

    template<typename... argument_types>
    void logger::error(std::string_view format_string, argument_types&&... arguments)
    {
        if (!_logger) return;
        _logger->error(format_string, std::forward<argument_types>(arguments)...);
    }

    template<typename... argument_types>
    void logger::critical(std::string_view format_string, argument_types&&... arguments)
    {
        if (!_logger) return;
        _logger->critical(format_string, std::forward<argument_types>(arguments)...);
    }
}
