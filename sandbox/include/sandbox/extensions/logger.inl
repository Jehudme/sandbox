#pragma once
#include "sandbox/core/engine.h"

namespace sandbox::extensions
{
    template<typename... argument_types>
    void logger::trace(std::string_view format_string, argument_types&&... arguments)
    {
        if (!enabled()) return;
        flecs::entity extension_logger_entity = _app->world.lookup("::extensions::logger");
        if (!extension_logger_entity.is_valid()) return;
        if (!extension_logger_entity.template has<std::unique_ptr<sandbox::logger>>()) return;
        const auto& logger_ptr = extension_logger_entity.template get<std::unique_ptr<sandbox::logger>>();
        if (!logger_ptr) return;
        logger_ptr->trace(format_string, std::forward<argument_types>(arguments)...);
    }

    template<typename... argument_types>
    void logger::debug(std::string_view format_string, argument_types&&... arguments)
    {
        if (!enabled()) return;
        flecs::entity extension_logger_entity = _app->world.lookup("::extensions::logger");
        if (!extension_logger_entity.is_valid()) return;
        if (!extension_logger_entity.template has<std::unique_ptr<sandbox::logger>>()) return;
        const auto& logger_ptr = extension_logger_entity.template get<std::unique_ptr<sandbox::logger>>();
        if (!logger_ptr) return;
        logger_ptr->debug(format_string, std::forward<argument_types>(arguments)...);
    }

    template<typename... argument_types>
    void logger::info(std::string_view format_string, argument_types&&... arguments)
    {
        if (!enabled()) return;
        flecs::entity extension_logger_entity = _app->world.lookup("::extensions::logger");
        if (!extension_logger_entity.is_valid()) return;
        if (!extension_logger_entity.template has<std::unique_ptr<sandbox::logger>>()) return;
        const auto& logger_ptr = extension_logger_entity.template get<std::unique_ptr<sandbox::logger>>();
        if (!logger_ptr) return;
        logger_ptr->info(format_string, std::forward<argument_types>(arguments)...);
    }

    template<typename... argument_types>
    void logger::warn(std::string_view format_string, argument_types&&... arguments)
    {
        if (!enabled()) return;
        flecs::entity extension_logger_entity = _app->world.lookup("::extensions::logger");
        if (!extension_logger_entity.is_valid()) return;
        if (!extension_logger_entity.template has<std::unique_ptr<sandbox::logger>>()) return;
        const auto& logger_ptr = extension_logger_entity.template get<std::unique_ptr<sandbox::logger>>();
        if (!logger_ptr) return;
        logger_ptr->warn(format_string, std::forward<argument_types>(arguments)...);
    }

    template<typename... argument_types>
    void logger::error(std::string_view format_string, argument_types&&... arguments)
    {
        if (!enabled()) return;
        flecs::entity extension_logger_entity = _app->world.lookup("::extensions::logger");
        if (!extension_logger_entity.is_valid()) return;
        if (!extension_logger_entity.template has<std::unique_ptr<sandbox::logger>>()) return;
        const auto& logger_ptr = extension_logger_entity.template get<std::unique_ptr<sandbox::logger>>();
        if (!logger_ptr) return;
        logger_ptr->error(format_string, std::forward<argument_types>(arguments)...);
    }

    template<typename... argument_types>
    void logger::critical(std::string_view format_string, argument_types&&... arguments)
    {
        if (!enabled()) return;
        flecs::entity extension_logger_entity = _app->world.lookup("::extensions::logger");
        if (!extension_logger_entity.is_valid()) return;
        if (!extension_logger_entity.template has<std::unique_ptr<sandbox::logger>>()) return;
        const auto& logger_ptr = extension_logger_entity.template get<std::unique_ptr<sandbox::logger>>();
        if (!logger_ptr) return;
        logger_ptr->critical(format_string, std::forward<argument_types>(arguments)...);
    }
}
