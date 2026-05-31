#pragma once

#include <utility>
#include <memory>

namespace sandbox::events {
    struct ChannelTag {};

    inline flecs::entity default_channel(flecs::world world) {
        flecs::entity default_channel_entity = world.entity("::sandbox::DefaultEventChannel");
        if (!default_channel_entity.has<ChannelTag>()) {
            default_channel_entity.add<ChannelTag>();
        }
        return default_channel_entity;
    }

    template <typename EventType>
    inline void publish(flecs::world world, const EventType& payload, flecs::entity channel) {
        flecs::entity channel_entity = channel.is_valid() ? channel : default_channel(world);

        if (!channel_entity.has<ChannelTag>()) {
            channel_entity.add<ChannelTag>();
        }

        world.event<EventType>()
            .entity(channel_entity)
            .template id<ChannelTag>()
            .ctx(payload)
            .emit();
    }

    template <typename EventType, typename Func>
    inline flecs::entity subscribe(flecs::world world, Func&& callback, flecs::entity channel) {
        flecs::entity channel_entity = channel.is_valid() ? channel : default_channel(world);

        if (!channel_entity.has<ChannelTag>()) {
            channel_entity.add<ChannelTag>();
        }

        return world.observer()
            .template event<EventType>()
            .template with<ChannelTag>().src(channel_entity)
            .run([callback_forward = std::forward<Func>(callback)](flecs::iter& iterator) {
                const EventType* payload_pointer = iterator.ctx<EventType>();
                if (!payload_pointer) return;

                callback_forward(*payload_pointer);
            });
    }

    template <typename EventType>
    inline void publish_async(flecs::world world, EventType payload, flecs::entity channel) {
        flecs::entity channel_entity = channel.is_valid() ? channel : default_channel(world);
        auto heap_payload = std::make_shared<EventType>(std::move(payload));

        world.defer([world, heap_payload, channel_entity]() {
            publish(world, *heap_payload, channel_entity);
        });
    }

} // namespace sandbox::events