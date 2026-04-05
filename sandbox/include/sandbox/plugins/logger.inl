#pragma once

namespace sandbox::plugins
{
    template<typename... argument_types>
    void logger::log(log_level lvl, std::string_view format_string, argument_types&&... arguments) const {
        std::string formatted_message = fmt::format(format_string, std::forward<argument_types>(arguments)...);
        _internal_log(lvl, formatted_message);
    }

    template<typename... argument_types>
    void logger::trace(std::string_view format_string, argument_types&&... arguments) const {
        log(log_level::trace, format_string, std::forward<argument_types>(arguments)...);
    }

    template<typename... argument_types>
    void logger::debug(std::string_view format_string, argument_types&&... arguments) const {
        log(log_level::debug, format_string, std::forward<argument_types>(arguments)...);
    }

    template<typename... argument_types>
    void logger::info(std::string_view format_string, argument_types&&... arguments) const {
        log(log_level::info, format_string, std::forward<argument_types>(arguments)...);
    }

    template<typename... argument_types>
    void logger::warn(std::string_view format_string, argument_types&&... arguments) const {
        log(log_level::warn, format_string, std::forward<argument_types>(arguments)...);
    }

    template<typename... argument_types>
    void logger::error(std::string_view format_string, argument_types&&... arguments) const {
        log(log_level::error, format_string, std::forward<argument_types>(arguments)...);
    }

    template<typename... argument_types>
    void logger::critical(std::string_view format_string, argument_types&&... arguments) const {
        log(log_level::critical, format_string, std::forward<argument_types>(arguments)...);
    }
}