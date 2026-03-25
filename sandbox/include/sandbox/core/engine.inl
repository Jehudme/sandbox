#pragma once

#include <flecs.h>

namespace sandbox
{
    template<typename derived_type>
    derived_type* engine::get_extension(std::string_view category)
    {
        std::string absolute_path = "::extensions::" + std::string(category);
        auto extension_entity = _world.lookup(absolute_path.c_str());

        if (extension_entity.is_valid() && extension_entity.template has<std::unique_ptr<sandbox::extension>>())
        {
            auto& extension_pointer = extension_entity.template get_mut<std::unique_ptr<sandbox::extension>>();
            return static_cast<derived_type*>(extension_pointer.get());
        }
        return nullptr;
    }

    template<typename... components>
    void engine::create_system(std::string_view name, std::string_view stage, auto&& configuration_lambda, auto&& logic_lambda)
    {
        std::string absolute_path = name.starts_with("::") ? std::string(name) : "::systems::" + std::string(name);
        auto system_builder = _world.template system<components...>(absolute_path.c_str());

        if (!stage.empty())
        {
            std::string stage_path = stage.starts_with("::") ? std::string(stage) : "::stages::" + std::string(stage);
            system_builder.kind(_world.entity(stage_path.c_str()));
        }

        configuration_lambda(system_builder);

        if constexpr (std::is_invocable_v<std::remove_reference_t<decltype(logic_lambda)>, flecs::iter&>)
        {
            system_builder.run(std::forward<decltype(logic_lambda)>(logic_lambda));
        }
        else
        {
            system_builder.each(std::forward<decltype(logic_lambda)>(logic_lambda));
        }

        _log(logger::level::debug, "engine: created system '{}'", absolute_path);
    }

    template<typename... components>
    void engine::create_system(std::string_view name, std::string_view stage, auto&& logic_lambda)
    {
        create_system<components...>(name, stage, [](auto&){}, std::forward<decltype(logic_lambda)>(logic_lambda));
    }

    template<typename... components>
    void engine::create_trigger(std::string_view name, auto&& configuration_lambda, auto&& logic_lambda)
    {
        std::string absolute_path = name.starts_with("::") ? std::string(name) : "::triggers::" + std::string(name);
        auto observer_builder = _world.template observer<components...>(absolute_path.c_str());

        configuration_lambda(observer_builder);

        if constexpr (std::is_invocable_v<std::remove_reference_t<decltype(logic_lambda)>, flecs::iter&>)
        {
            observer_builder.run(std::forward<decltype(logic_lambda)>(logic_lambda));
        }
        else
        {
            observer_builder.each(std::forward<decltype(logic_lambda)>(logic_lambda));
        }

        _log(logger::level::debug, "engine: created trigger '{}'", absolute_path);
    }

    template<typename base_type, typename... component_args>
    void engine::create_object(std::string_view name, component_args&&... initialization_arguments)
    {
        std::string absolute_path = name.starts_with("::") ? std::string(name) : "::objects::" + std::string(name);
        auto object_entity = _world.entity(absolute_path.c_str());

        object_entity.template set<base_type>({std::forward<component_args>(initialization_arguments)...});

        _log(logger::level::debug, "engine: created object '{}'", absolute_path);
    }

    template<typename base_type>
    base_type* engine::get_object(std::string_view name)
    {
        std::string absolute_path = name.starts_with("::") ? std::string(name) : "::objects::" + std::string(name);
        auto object_entity = _world.lookup(absolute_path.c_str());

        if (object_entity.is_valid() && object_entity.template has<base_type>())
        {
            return &object_entity.template get_mut<base_type>();
        }
        return nullptr;
    }

    template <typename ... argument_types>
    void engine::_log(logger::level level, std::string_view format_string, argument_types&&... arguments)
    {
        // Important: _log is used during shutdown too; never throw from here.
        auto logger_entity = _world.lookup("::internals::engine_logger");
        if (!logger_entity.is_valid() || !logger_entity.template has<std::unique_ptr<sandbox::logger>>())
            return;

        auto& logger_ptr = logger_entity.template get_mut<std::unique_ptr<sandbox::logger>>();
        if (!logger_ptr)
            return;

        logger_ptr->log(level, format_string, std::forward<argument_types>(arguments)...);
    }
}