# 5. How to Use the Event Bus

*This guide shows you how to decouple systems using the Event Bus and broadcast FlatBuffer payloads safely across the ABI wall.*

**Conclusion first:** To use the Event Bus, you define a FlatBuffer schema (`.fbs`), compile it to C++, construct the FlatBuffer using the generated builder, pack the raw bytes into a `sandbox_payload`, and emit it using `publish_raw()`. Subscribers use `subscribe_raw()` to cast the raw payload back into the FlatBuffer.

## Creating and Broadcasting a FlatBuffer Event

In this example, we want to broadcast an event containing a complex `std::string` that notifies other plugins a player joined. Because `std::string` cannot cross the DLL boundary safely, we use a FlatBuffer.

### Step 1: The Schema (`player_events.fbs`)
```flatbuffers
namespace my_game::schemas;

table PlayerJoinedEvent {
  player_id: uint64;
  username: string;
}

root_type PlayerJoinedEvent;
```
*(Compile this using `flatc --cpp player_events.fbs`)*

### Step 2: Publishing the Event

```cpp
#include <sandbox/utilities/events.h>
#include "player_events_generated.h"

void notify_player_join(flecs::world& ecs, uint64_t id, const std::string& name) {
    // 1. Build the FlatBuffer
    flatbuffers::FlatBufferBuilder builder;
    auto name_str = builder.CreateString(name);
    
    my_game::schemas::PlayerJoinedEventBuilder ev_builder(builder);
    ev_builder.add_player_id(id);
    ev_builder.add_username(name_str);
    builder.Finish(ev_builder.Finish());

    // 2. Pack the raw bytes into the ABI-safe payload struct
    sandbox::abi::flatbuffer_payload payload;
    payload.bytes = builder.GetBufferPointer();
    payload.size = builder.GetSize();
    payload.free_func = nullptr; // Memory is stack-owned by the builder in this sync-call

    // 3. Emit synchronously using an arbitrary unique ID for the event type
    uint64_t EVENT_PLAYER_JOIN = 10001; 
    sandbox::events::publish_raw(ecs, EVENT_PLAYER_JOIN, payload);
}
```

### Step 3: Subscribing to the Event

Any other plugin can now listen for this event without knowing the implementation details of the publisher!

```cpp
#include <sandbox/utilities/events.h>
#include "player_events_generated.h"
#include <iostream>

struct ui_module {
    ui_module(flecs::world& ecs) {
        uint64_t EVENT_PLAYER_JOIN = 10001; 

        // Subscribe to the event
        sandbox::events::subscribe_raw(ecs, EVENT_PLAYER_JOIN, [](const sandbox::abi::flatbuffer_payload& payload) {
            
            // 1. Unpack the FlatBuffer from the raw bytes
            auto* fb_event = flatbuffers::GetRoot<my_game::schemas::PlayerJoinedEvent>(payload.bytes);
            
            if (fb_event) {
                // 2. Use the data safely!
                std::cout << "UI: Player " << fb_event->username()->c_str() 
                          << " (ID: " << fb_event->player_id() << ") has joined the game!\n";
            }
        });
    }
};
```
