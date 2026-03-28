#pragma once

#include "sandbox/diagnostics/logger.h"
#include "sandbox/ecs/systems_ext.h"
#include "sandbox/core/engine.h"

#include <type_traits>

namespace sandbox::extensions
{
    template<typename event_type>
    void events::publish(event_type&& event_data)
    {
        using bare_type = std::remove_cvref_t<event_type>;
        _app->world.template component<bare_type>();

        // 1. Create the transient source entity and attach the payload.
        //    This guarantees a non-empty table, preventing the 'desc->table != NULL' crash.
        //    The entity is tagged for automatic cleanup at end-of-frame (PostUpdate).
        auto event_entity = _app->world.entity();
        event_entity.template add<transient_event_tag>();
        event_entity.template set<bare_type>(std::forward<event_type>(event_data));

        // 2. Emit the event immediately. All observers are called synchronously
        //    before this function returns (Model A – immediate delivery).
        _app->world.template event<bare_type>()
            .template id<bare_type>()
            .entity(event_entity)
            .emit();
    }

    template<typename event_type>
    void events::subscribe(std::string_view name, auto&& callback)
    {
        // Store the observer at ::events::<name> so that destroy/enable/disable
        // can reliably look it up via the same path.
        const std::string absolute_path = "::events::" + std::string(name);
        _app->world.template component<event_type>();

        _app->world.template observer<event_type>(absolute_path.c_str())
            .template event<event_type>()
            .run([captured_callback = std::forward<decltype(callback)>(callback)](flecs::iter& it) {
                if constexpr (std::is_empty_v<event_type>)
                {
                    for (auto _ : it)
                    {
                        event_type evt{};
                        captured_callback(evt);
                    }
                }
                else
                {
                    auto events = it.template field<event_type>(0);
                    for (auto i : it)
                    {
                        captured_callback(events[i]);
                    }
                }
            });
    }
}
