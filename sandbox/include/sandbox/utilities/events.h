#pragma once

#include <flecs.h>
#include "sandbox/core/ecs.h"

namespace sandbox::events {

    template <typename EventType>
    void publish(flecs::world world, const EventType& payload, flecs::entity channel = flecs::entity());

    template <typename EventType>
    void publish_async(flecs::world world, EventType payload, flecs::entity channel = flecs::entity());

    template <typename EventType, typename Func>
    flecs::entity subscribe(flecs::world world, Func&& callback, flecs::entity channel = flecs::entity());

}

#include "../../../src/utilities/events.inl"