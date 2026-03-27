#pragma once
#include "sandbox/core/engine.h"

namespace sandbox::extensions
{
    template<typename... argument_types>
    void logger::trace(std::string_view format_string, argument_types&&... arguments)
    {
        if (!enabled()) return;
        _app->world.entity("::extensions::logger").template get<std::unique_ptr<sandbox::logger>>()->trace(format_string, std::forward<argument_types>(arguments)...);
    }

    template<typename... argument_types>
    void logger::debug(std::string_view format_string, argument_types&&... arguments)
    {
        if (!enabled()) return;
        _app->world.entity("::extensions::logger").template get<std::unique_ptr<sandbox::logger>>()->debug(format_string, std::forward<argument_types>(arguments)...);
    }

    template<typename... argument_types>
    void logger::info(std::string_view format_string, argument_types&&... arguments)
    {
        if (!enabled()) return;
        _app->world.entity("::extensions::logger").template get<std::unique_ptr<sandbox::logger>>()->info(format_string, std::forward<argument_types>(arguments)...);
    }

    template<typename... argument_types>
    void logger::warn(std::string_view format_string, argument_types&&... arguments)
    {
        if (!enabled()) return;
        _app->world.entity("::extensions::logger").template get<std::unique_ptr<sandbox::logger>>()->warn(format_string, std::forward<argument_types>(arguments)...);
    }

    template<typename... argument_types>
    void logger::error(std::string_view format_string, argument_types&&... arguments)
    {
        if (!enabled()) return;
        _app->world.entity("::extensions::logger").template get<std::unique_ptr<sandbox::logger>>()->error(format_string, std::forward<argument_types>(arguments)...);
    }

    template<typename... argument_types>
    void logger::critical(std::string_view format_string, argument_types&&... arguments)
    {
        if (!enabled()) return;
        _app->world.entity("::extensions::logger").template get<std::unique_ptr<sandbox::logger>>()->critical(format_string, std::forward<argument_types>(arguments)...);
    }
}
