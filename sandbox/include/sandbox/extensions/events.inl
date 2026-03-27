#pragma once

#include "sandbox/core/engine.h"

namespace sandbox::extensions
{
    template<typename event_type>
    void events::publish(event_type event_data)
    {
        if (!_app)
        {
            return;
        }

        _app->_log(sandbox::logger::level::debug, "extensions::events: publishing event");

        auto event_entity = _app->world.entity();
        event_entity.set<event_type>(std::move(event_data));
    }

    template<typename event_type>
    void events::subscribe(std::string_view name, auto&& callback)
    {
        if (!_app)
        {
            return;
        }

        const std::string absolute_path = "::events::" + std::string(name);
        _app->_log(sandbox::logger::level::info, "extensions::events: subscribing immediate '{}' to event", absolute_path);

        _app->world.template observer<event_type>(absolute_path.c_str())
            .event(flecs::OnSet)
            .each(std::forward<decltype(callback)>(callback));
    }

    template<typename event_type>
    void events::subscribe(std::string_view name, std::string_view stage, auto&& callback)
    {
        if (!_app)
        {
            return;
        }

        const std::string absolute_path = "::events::" + std::string(name);
        _app->_log(sandbox::logger::level::info, "extensions::events: subscribing staged '{}' in stage '{}' to event", absolute_path, stage);

        auto system_builder = _app->world.template system<event_type>(absolute_path.c_str());

        if (!stage.empty())
        {
            const std::string stage_path = stage.starts_with("::") ? std::string(stage) : "::stages::" + std::string(stage);
            auto stage_entity = _app->world.lookup(stage_path.c_str());

            if (!stage_entity.is_valid())
            {
                _app->_log(sandbox::logger::level::warn, "extensions::events: stage '{}' not found for subscriber '{}'", stage_path, absolute_path);
            }
            else
            {
                system_builder.kind(stage_entity);
            }
        }

        system_builder.each([captured_callback = std::forward<decltype(callback)>(callback)](flecs::entity entity, event_type& event_data) {
            captured_callback(entity, event_data);
            entity.template remove<event_type>();
        });
    }
}
