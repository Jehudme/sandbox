#pragma once

#include <utility>

namespace sandbox::events {

    // =========================================================================
    // Internal Routing Infrastructure
    // =========================================================================

    struct ChannelTag {};

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

    inline void publish_raw(flecs::world world, uint64_t event_id, const sandbox::abi::flatbuffer_payload& payload, flecs::entity channel) {
        flecs::entity channel_entity = channel.is_valid() ? channel : default_channel(world);

        if (!channel_entity.has<ChannelTag>()) {
            channel_entity.add<ChannelTag>();
        }

        world.event(world.entity(event_id))
            .entity(channel_entity)
            .id(world.id<ChannelTag>())
            .ctx(const_cast<sandbox::abi::flatbuffer_payload*>(&payload))
            .emit();
    }

    inline void publish_raw_async(flecs::world world, uint64_t event_id, sandbox::abi::flatbuffer_payload payload, flecs::entity channel) {
        world.defer([world, event_id, payload, channel]() {
            publish_raw(world, event_id, payload, channel);
        });
    }

    // =========================================================================
    // Subscriber Implementation
    // =========================================================================

    template <typename Func>
    inline flecs::entity subscribe_raw(flecs::world world, uint64_t event_id, Func&& callback, flecs::entity channel) {
        flecs::entity channel_entity = channel.is_valid() ? channel : default_channel(world);

        if (!channel_entity.has<ChannelTag>()) {
            channel_entity.add<ChannelTag>();
        }

        flecs::entity previous_scope = world.set_scope(0);

        flecs::entity observer = world.observer()
            .event(world.entity(event_id))
            .template with<ChannelTag>().src(channel_entity)
            .run([callback_forward = std::forward<Func>(callback)](flecs::iter& iterator) {
                const sandbox::abi::flatbuffer_payload* payload_pointer = static_cast<const sandbox::abi::flatbuffer_payload*>(iterator.param());
                if (!payload_pointer) return;
                callback_forward(*payload_pointer);
            });

        world.set_scope(previous_scope);

        return observer;
    }

} // namespace sandbox::events
