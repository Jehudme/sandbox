#pragma once

namespace sandbox::events {

    template<typename... ArgumentTypes>
    inline log::log(const char* file, int line, level severity, std::optional<bool> throw_override, std::format_string<ArgumentTypes...> format_string, ArgumentTypes&&... arguments)
        : message(std::format(format_string, std::forward<ArgumentTypes>(arguments)...))
        , log_level(severity)
        , source_file(file)
        , source_line(line)
        , throw_on_error_override(throw_override) // Set the override field
    {
    }

} // namespace sandbox::events