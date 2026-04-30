#pragma once
#include "flecs/addons/cpp/entity.hpp"
#include "sandbox/core/engine.h"
#include "sandbox/core/plugin.h"

namespace sandbox
{
    // 1. Define an empty struct to act as our ECS term/filter
    struct global_event_bus {};

    class signals : public plugin
    {
    public:
        signals(engine& context);
        ~signals();

        template<typename event_type>
        void publish(const event_type& event);

        template<typename event_type>
        entity subscribe(std::function<void(const event_type&)> callback);

    private:
        void initialize() override;
        void finalize() override;

        // 2. A dedicated internal entity to act as our event router
        flecs::entity m_bus_entity;
    };

    // --- Template Implementations ---

    template<typename event_type>
    void signals::publish(const event_type& event)
    {
        // Tell Flecs: Emit this event specifically targeting the 'global_event_bus' tag!
        context.ecs.event<event_type>()
             .template id<global_event_bus>() // <-- The missing link!
             .entity(m_bus_entity)
             .ctx(event)
             .emit();
    }

    template<typename event_type>
    entity signals::subscribe(std::function<void(const event_type&)> callback)
    {
        return context.ecs.observer()
            .template event<event_type>()
            .template with<global_event_bus>()
            .each([callback](flecs::iter& it, size_t /* index */) {
                if (it.param()) {
                    callback(*static_cast<const event_type*>(it.param()));
                }
            });
    }
}