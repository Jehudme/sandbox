#pragma once
namespace sandbox::extensions {

    template<typename... args>
    void logger::log(level log_level, std::format_string<args...> format_string, args&&... arguments) const {
        implementation_log(log_level, std::format(format_string, std::forward<args>(arguments)...));
    }

    template<typename... args>
    void logger::trace(std::format_string<args...> format_string, args&&... arguments) const {
        log(level::trace, format_string, std::forward<args>(arguments)...);
    }

    template<typename... args>
    void logger::debug(std::format_string<args...> format_string, args&&... arguments) const {
        log(level::debug, format_string, std::forward<args>(arguments)...);
    }

    template<typename... args>
    void logger::info(std::format_string<args...> format_string, args&&... arguments) const {
        log(level::info, format_string, std::forward<args>(arguments)...);
    }

    template<typename... args>
    void logger::warn(std::format_string<args...> format_string, args&&... arguments) const {
        log(level::warn, format_string, std::forward<args>(arguments)...);
    }

    template<typename... args>
    void logger::critical(std::format_string<args...> format_string, args&&... arguments) const {
        log(level::critical, format_string, std::forward<args>(arguments)...);
    }

}
