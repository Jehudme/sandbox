# How-To: Use the Event Bus

The Event Bus allows cross-plugin communication without direct C++ linkage. Payloads are defined using FlatBuffers.

## 1. Subscribing to an Event

Use `sandbox::subscribe`.

```cpp
#include <sandbox/utilities/events.h>
#include "my_event_generated.h"

int32_t my_plugin_activate(flecs::world& ecs, const module_info* info) {
    sandbox::subscribe(ecs, "player_jump_event", [](const uint8_t* payload_data, size_t size) {
        auto fb = flatbuffers::GetRoot<MyGame::PlayerJump>(payload_data);
        float force = fb->force();
        // Handle jump logic...
    });
    return 0;
}
```

## 2. Publishing an Event

Use `sandbox::publish`.

```cpp
#include <sandbox/utilities/events.h>
#include "my_event_generated.h"

void trigger_jump(flecs::world& ecs) {
    flatbuffers::FlatBufferBuilder builder;
    auto jump_event = MyGame::CreatePlayerJump(builder, 15.0f);
    builder.Finish(jump_event);

    sandbox::publish(ecs, "player_jump_event", builder.GetBufferPointer(), builder.GetSize());
}
```
