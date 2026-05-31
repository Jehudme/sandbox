#pragma once

#include <flecs.h>
#include "sandbox/core/ecs.h"

namespace sandbox::events {

    struct GlobalBusTag {};

    // Returns a stable entity used as the "channel"
    inline flecs::entity bus(flecs::world ecs);

    // Synchronous publish: safe to pass stack payload
    template <typename EventType>
    void publish(flecs::world ecs, const EventType& payload);

    // Asynchronous publish: payload must outlive delivery, so we heap-own it
    template <typename EventType>
    void publish_async(flecs::world ecs, EventType payload);

    // Subscribe to EventType on the global bus
    template <typename EventType, typename Func>
    flecs::entity subscribe(flecs::world ecs, Func&& callback);

} // namespace sandbox::events

#include "events.inl"