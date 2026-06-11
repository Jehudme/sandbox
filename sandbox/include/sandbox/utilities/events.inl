#pragma once

#include <utility>

namespace sandbox::events {

    // ============================================================================
    // Internal Routing Infrastructure
    // ============================================================================

    // The component tag used to satisfy the observer's .with<channel_tag>() filter
    struct channel_tag {};

    // Helper to lazily construct and retrieve the global default event bus
    inline flecs::entity default_channel(flecs::world world) {
        flecs::entity bus = world.entity("::sandbox::events::DefaultEventBus");
        if (!bus.has<channel_tag>()) {
            bus.add<channel_tag>();
        }
        return bus;
    }

    // ============================================================================
    // Publisher Implementations
    // ============================================================================

    template <typename EventType>
        inline void publish(flecs::world world, const EventType& payload, flecs::entity channel) {
        flecs::entity channel_entity = channel.is_valid() ? channel : default_channel(world);

        if (!channel_entity.has<channel_tag>()) {
            channel_entity.add<channel_tag>();
        }

        // Emit the event ON the channel entity, cleanly linked to the channel_tag filter.
        // FIXED: Flecs Typed API expects a reference, not a pointer!
        world.event<EventType>()
            .entity(channel_entity)
            .id(world.id<channel_tag>())
            .ctx(payload) // <-- Just pass the reference directly!
            .emit();
    }

    template <typename EventType>
    inline void publish_async(flecs::world world, EventType payload, flecs::entity channel) {
        // For asynchronous publishing, we capture the payload by value in a lambda.
        // Flecs will safely defer this execution until it processes its command queue
        // (usually at the end of the frame), avoiding dangling pointers.
        world.defer([world, payload_copy = std::move(payload), channel]() mutable {
            publish<EventType>(world, payload_copy, channel);
        });
    }

    // ============================================================================
    // Subscriber Implementation
    // ============================================================================

    template <typename EventType, typename Func>
    inline flecs::entity subscribe(flecs::world world, Func&& callback, flecs::entity channel) {
        flecs::entity channel_entity = channel.is_valid() ? channel : default_channel(world);

        if (!channel_entity.has<channel_tag>()) {
            channel_entity.add<channel_tag>();
        }

        // 1. Break out of any active Module Namespace trap so this observer listens globally
        flecs::entity previous_scope = world.set_scope(0);

        // 2. Build the exact query structure required to catch the routed event
        flecs::entity observer = world.observer()
            .template event<EventType>()
            .template with<channel_tag>()
            .run([callback_forward = std::forward<Func>(callback)](flecs::iter& iterator) {

                // Extract the data pointer using param() which pairs with the publisher's ctx()
                const EventType* payload_pointer = static_cast<const EventType*>(iterator.param());

                if (!payload_pointer) return;

                callback_forward(*payload_pointer);
            });

        // 3. Restore the original namespace scope so future module logic remains contained
        world.set_scope(previous_scope);

        return observer;
    }

} // namespace sandbox::events