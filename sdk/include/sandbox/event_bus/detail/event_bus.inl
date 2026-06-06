#pragma once

#include <utility>

namespace sandbox::events {

    // =========================================================================
    // Internal Routing Infrastructure
    // =========================================================================

    // The component tag used to satisfy the observer's .with<ChannelTag>() filter
    struct ChannelTag {};

    // Helper to lazily construct and retrieve the global default event bus
    inline flecs::entity default_channel(flecs::world world) {
        flecs::entity bus = world.entity("::sandbox::events::DefaultEventBus");
        if (!bus.has<ChannelTag>()) {
            bus.add<ChannelTag>();
        }
        return bus;
    }

    // =========================================================================
    // Publisher Implementations
    // =========================================================================

    template <typename EventType>
    inline void publish(flecs::world world, const EventType& payload, flecs::entity channel) {
        flecs::entity channel_entity = channel.is_valid() ? channel : default_channel(world);

        if (!channel_entity.has<ChannelTag>()) {
            channel_entity.add<ChannelTag>();
        }

        world.event<EventType>()
            .entity(channel_entity)
            .id(world.id<ChannelTag>())
            .ctx(payload)
            .emit();
    }

    template <typename EventType>
    inline void publish_async(flecs::world world, EventType payload, flecs::entity channel) {
        world.defer([world, payload_copy = std::move(payload), channel]() mutable {
            publish<EventType>(world, payload_copy, channel);
        });
    }

    // =========================================================================
    // Subscriber Implementation
    // =========================================================================

    template <typename EventType, typename Func>
    inline flecs::entity subscribe(flecs::world world, Func&& callback, flecs::entity channel) {
        flecs::entity channel_entity = channel.is_valid() ? channel : default_channel(world);

        if (!channel_entity.has<ChannelTag>()) {
            channel_entity.add<ChannelTag>();
        }

        // Break out of any active Module Namespace trap so this observer listens globally
        flecs::entity previous_scope = world.set_scope(0);

        flecs::entity observer = world.observer()
            .template event<EventType>()
            .template with<ChannelTag>().src(channel_entity)
            .run([callback_forward = std::forward<Func>(callback)](flecs::iter& iterator) {
                const EventType* payload_pointer = static_cast<const EventType*>(iterator.param());
                if (!payload_pointer) return;
                callback_forward(*payload_pointer);
            });

        // Restore the original namespace scope
        world.set_scope(previous_scope);

        return observer;
    }

} // namespace sandbox::events
