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

        _app->world.template event<bare_type>()
            .template id<bare_type>()
            .ctx(std::forward<event_type>(event_data))
            .emit();
    }

    template<typename event_type>
    void events::subscribe(std::string_view name, auto&& callback)
    {
        const std::string absolute_path = "::events::" + std::string(name);
        _app->world.template component<event_type>();

        _app->world.template observer<event_type>(absolute_path.c_str())
            .template event<event_type>()
            .run([captured_callback = std::forward<decltype(callback)>(callback)](flecs::iter& it) {
                if constexpr (std::is_empty_v<event_type>)
                {
                    while (it.next())
                    {
                        event_type evt{};
                        captured_callback(evt);
                    }
                }
                else
                {
                    while (it.next())
                    {
                        if (auto* payload = it.param<event_type>())
                        {
                            captured_callback(*payload);
                        }
                    }
                }
            });
    }
}
