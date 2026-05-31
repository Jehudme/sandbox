#pragma once

namespace sandbox::events {

    template<typename... ArgumentTypes>
    inline log::log(std::format_string<ArgumentTypes...> format_string, level severity, ArgumentTypes&&... arguments)
        : message(std::format(format_string, std::forward<ArgumentTypes>(arguments)...)), log_level(severity)
    {
    }

} // namespace sandbox::events