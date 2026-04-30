#pragma once
#include "flecs/addons/cpp/entity.hpp"
#include "sandbox/core/engine.h"
#include "sandbox/core/plugin.h"

namespace sandbox
{
    class dispatcher : public plugin
    {
    public:

        void initialize() override;
        void finalize() override;

        /**
         * @brief Publishes a global event to the ECS world.
         */
        template<typename event_type>
        void publish(const event_type& event);

        /**
         * @brief Subscribes to an event and returns the Observer entity.
         * @return flecs::entity The entity representing this subscription.
         */
        template<typename event_type>
        entity subscribe(std::function<void(const event_type&)> callback);
    };

    // --- Template Implementations (Outside declaration) ---

    template<typename event_type>
    void dispatcher::publish(const event_type& event)
    {
        // Emit the event via the Flecs world
        context.ecs.event<event_type>()
             .set(event)
             .emit();
    }

    template<typename event_type>
    entity dispatcher::subscribe(std::function<void(const event_type&)> callback)
    {
        return context.ecs.observer<event_type>()
            .template event<event_type>()
            .yield_existing(true)
            .each([callback](const event_type& data) {
                callback(data);
            });
    }
}