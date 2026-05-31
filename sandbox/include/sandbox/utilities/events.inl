#pragma once

#include <type_traits>
#include <utility>

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
        flecs::entity channel_entity = bus(world);

        world.event<EventType>()
            .entity(channel_entity)
            .template id<GlobalBusTag>()
            .ctx(payload)
            .emit();
    }

    template <typename EventType, typename Func>
    inline flecs::entity subscribe(flecs::world world, Func&& callback) {
        world.component<EventType>(); // Safe main-thread component registration

        return world.observer()
            .template event<EventType>()
            .template with<GlobalBusTag>()
            .run([callback_forward = std::forward<Func>(callback)](flecs::iter& iterator) {
                const void* payload_pointer = iterator.param();
                if (!payload_pointer) return;

                callback_forward(*static_cast<const EventType*>(payload_pointer));
            });
    }

    template <typename EventType>
    inline void publish_async(flecs::world world, EventType payload) {
        auto* heap_payload = new EventType(std::move(payload));

        world.defer([world, heap_payload]() {
            publish(world, *heap_payload);
            delete heap_payload;
        });
    }

} // namespace sandbox::events