#pragma once

#include <flecs.h>
#include "sandbox/core/ecs.h"

namespace sandbox::events {

    /// @brief Publishes an event synchronously to the ECS world or a specific channel.
    template <typename EventType>
    void publish(flecs::world world, const EventType& payload, flecs::entity channel = flecs::entity());

    /// @brief Queues an event to be published asynchronously during the next ECS pipeline execution.
    template <typename EventType>
    void publish_async(flecs::world world, EventType payload, flecs::entity channel = flecs::entity());

    /// @brief Subscribes a callback to a specific event type, optionally filtered by a channel.
    template <typename EventType, typename Func>
    flecs::entity subscribe(flecs::world world, Func&& callback, flecs::entity channel = flecs::entity());

}

#include "detail/event_bus.inl"
