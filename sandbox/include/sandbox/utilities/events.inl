#pragma once

#include <utility>
#include <memory>

namespace sandbox::events {

    inline flecs::entity bus(flecs::world world) {
        flecs::entity bus_entity = world.entity("sandbox::GlobalEventBus");
        if (!bus_entity.has<GlobalBusTag>()) {
            bus_entity.add<GlobalBusTag>();
        }
        return bus_entity;
    }

    template <typename EventType>
    inline void publish(flecs::world world, const EventType& payload) {
        world.event<EventType>()
            .entity(bus(world))
            .template id<GlobalBusTag>()
            .ctx(payload)
            .emit();
    }

    template <typename EventType, typename Func>
    inline flecs::entity subscribe(flecs::world world, Func&& callback) {
        return world.observer()
            .template event<EventType>()
            .template with<GlobalBusTag>()
            .run([callback_forward = std::forward<Func>(callback)](flecs::iter& iterator) {
                const EventType* payload_pointer = iterator.ctx<EventType>();
                if (!payload_pointer) return;

                callback_forward(*payload_pointer);
            });
    }

    template <typename EventType>
    inline void publish_async(flecs::world world, EventType payload) {
        auto heap_payload = std::make_shared<EventType>(std::move(payload));

        world.defer([world, heap_payload]() {
            publish(world, *heap_payload);
        });
    }

} // namespace sandbox::events