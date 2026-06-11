# 5.  The Event Bus Deep Dive

The Sandbox Engine uses an Event Bus to handle communication across dynamic boundaries safely.

## Decoupling Subsystems
Why use an Event Bus instead of direct module-to-module function calls?
When Plugin A requires Plugin B, it creates a hard dependency. If Plugin B changes its ABI layout or is unloaded, Plugin A crashes.
The Event Bus decouples systems entirely. Plugin A simply broadcasts an "Entity Damaged" event. It does not care if Plugin B (the Audio System) or Plugin C (the UI System) receives it. If neither exist, the event drops silently and safely.

## Publishing & Subscribing Internals

### The `ChannelTag` System
In `event_bus.inl`, the engine routes messages via ECS entities functioning as channels.
If a channel is not explicitly provided, the engine defaults to `::sandbox::events::DefaultEventBus`.
It ensures the channel entity is tagged with `ChannelTag` before creating an observer.

### Payload Serialization (FlatBuffers)
To broadcast complex types, the engine expects standard C++ code to wrap data using FlatBuffers.
The serialized bytes are handed off as a `sandbox::abi::flatbuffer_payload`.

**Synchronous Publishing:**
`publish_raw` immediately invokes the ECS observer triggers via `world.event(...).emit()`.

**Asynchronous Publishing:**
`publish_raw_async` uses `world.defer()`. This queues the lambda containing the `publish_raw` call until the current ECS system tick finishes processing, avoiding race conditions and iterator invalidations.

### Listening (`subscribe_raw`)
`subscribe_raw` creates a persistent Flecs `observer()`. It specifically filters by the event ID and the `ChannelTag` source. 
The underlying lambda casts the `iterator.param()` back into a `const sandbox::abi::flatbuffer_payload*`, which the plugin can then safely decode.
