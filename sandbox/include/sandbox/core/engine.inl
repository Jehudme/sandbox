#pragma once

#include <flecs.h>

namespace sandbox
{
    template<typename derived_type>
    derived_type* engine::get_extension(std::string_view category)
    {
        std::string absolute_path = "::extensions::" + std::string(category);
        auto extension_entity = world.lookup(absolute_path.c_str());

        if (extension_entity.is_valid() && extension_entity.template has<std::unique_ptr<sandbox::extension>>())
        {
            auto& extension_pointer = extension_entity.template get_mut<std::unique_ptr<sandbox::extension>>();
            return static_cast<derived_type*>(extension_pointer.get());
        }
        return nullptr;
    }

    template <typename ... argument_types>
    void engine::_log(logger::level level, std::string_view format_string, argument_types&&... arguments)
    {
        auto logger_entity = world.lookup("::internals::engine_logger");
        if (!logger_entity.is_valid() || !logger_entity.template has<std::unique_ptr<sandbox::logger>>())
            return;

        auto& logger_ptr = logger_entity.template get_mut<std::unique_ptr<sandbox::logger>>();
        if (!logger_ptr)
            return;

        logger_ptr->log(level, format_string, std::forward<argument_types>(arguments)...);
    }
}