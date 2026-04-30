#pragma once

#include <string_view>
#include "flecs/addons/cpp/entity.hpp"
#include "sandbox/core/engine.h"
#include "sandbox/core/plugin.h"

namespace sandbox
{
    // 1. The universal tag used to identify ANY event channel (default or custom)
    struct signal_channel {};

    class signals : public plugin
    {
    public:
        signals(engine& context);
        ~signals();

        // Creates a custom isolated channel entity
        flecs::entity create_channel(std::string_view name);

        // Publishes an event. If 'channel' is omitted/null, uses the default bus.
        template<typename event_type>
        void publish(const event_type& event, flecs::entity channel = {});

        // Subscribes to an event. If 'channel' is omitted/null, uses the default bus.
        template<typename event_type>
        flecs::entity subscribe(std::function<void(const event_type&)> callback, flecs::entity channel = {});

    private:
        void initialize() override;
        void finalize() override;

        // 2. The fallback/default event router
        flecs::entity m_default_bus;
    };

    // --- Template Implementations ---

    template<typename event_type>
    void signals::publish(const event_type& event, flecs::entity channel)
    {
        // 3. Fallback: If no valid channel is provided, route to the default bus
        flecs::entity target_bus = channel ? channel : m_default_bus;

        context.ecs.event<event_type>()
             .template id<signal_channel>() // Broadcast on the signal frequency
             .entity(target_bus)            // Broadcast from the targeted tower
             .ctx(event)
             .emit();
    }

    template<typename event_type>
    flecs::entity signals::subscribe(std::function<void(const event_type&)> callback, flecs::entity channel)
    {
        // 4. Fallback: If no valid channel is provided, listen to the default bus
        flecs::entity target_bus = channel ? channel : m_default_bus;

        return context.ecs.observer()
            .template event<event_type>()
            .template with<signal_channel>()
            .each([callback, target_bus](flecs::iter& it, size_t index) {
                // 5. Safety Filter: Ensure we only process events emitted from our specific target bus
                // This prevents observers on different channels from hearing each other
                if (it.entity(index) == target_bus) {
                    if (it.param()) {
                        callback(*static_cast<const event_type*>(it.param()));
                    }
                }
            });
    }
}