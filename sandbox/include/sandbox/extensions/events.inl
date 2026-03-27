#pragma once

#include "sandbox/extensions/logger.h"
#include "sandbox/extensions/systems.h"
#include "sandbox/core/engine.h"

namespace sandbox::extensions
{
    template<typename event_type>
    void events::publish(event_type event_data)
    {
        _app->world.template component<event_type>();

        // 1. Create the source entity and attach the payload (data)
        // This guarantees a valid table, preventing the 'desc->table != NULL' crash.
        auto event_entity = _app->world.entity();
        event_entity.template add<transient_event_tag>();
        event_entity.template set<event_type>(std::move(event_data));

        // 2. Emit the custom event exactly like the official example
        // - Event Kind: event_type
        // - Component ID: event_type
        // - Source: event_entity
        _app->world.template event<event_type>()
            .template id<event_type>()
            .entity(event_entity)
            .emit();
    }

    template<typename event_type>
    void events::subscribe(std::string_view name, auto&& callback)
    {
        const std::string absolute_path = "::events::" + std::string(name);
        _app->world.template component<event_type>();

        // 3. Observer matches the exact pattern of the official example
        _app->world.template observer<event_type>(absolute_path.c_str())
            .template event<event_type>()
            .each([captured_callback = std::forward<decltype(callback)>(callback)](flecs::iter& it, size_t index, event_type& data) {
                // We extract the data directly from the matched entity component
                captured_callback(data);
            });
    }

    template<typename event_type>
    void events::subscribe(std::string_view name, std::string_view stage, auto&& callback)
    {
        auto* systems_extension = _app->get_systems();
        if (systems_extension)
        {
            _app->world.template component<event_type>();

            systems_extension->create<event_type>(
                name,
                stage,
                [captured_callback = std::forward<decltype(callback)>(callback)](event_type& event_data) {
                    captured_callback(event_data);
                }
            );
        }
    }
}