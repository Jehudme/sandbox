#pragma once

#include <flecs.h>
#include "sandbox/core/ecs.h"
#include <sandbox/core/service_macro.h>
#include <cstdint>

namespace sandbox::events {

    /// @brief Publishes an event synchronously to the ECS world or a specific channel.
    void publish_raw(flecs::world world, uint64_t event_id, const sandbox_payload& payload, flecs::entity channel = flecs::entity());

    /// @brief Queues an event to be published asynchronously during the next ECS pipeline execution.
    void publish_raw_async(flecs::world world, uint64_t event_id, sandbox_payload payload, flecs::entity channel = flecs::entity());

    /// @brief Subscribes a callback to a specific event type, optionally filtered by a channel.
    template <typename Func>
    flecs::entity subscribe_raw(flecs::world world, uint64_t event_id, Func&& callback, flecs::entity channel = flecs::entity());

}

#include "detail/event_bus.inl"
