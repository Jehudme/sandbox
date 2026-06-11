# Tutorial: ECS Systems & Components

The Sandbox Engine uses [Flecs](https://github.com/SanderMertens/flecs), an extremely fast Entity Component System (ECS).

## Step 1: Defining a Component

Components are standard C++ structs.

```cpp
struct Transform {
    float x;
    float y;
};
```

## Step 2: Registering in a Plugin

In your `activate` function, register the component and define a system that operates on it.

```cpp
#include <sandbox/core/plugin.h>

struct Transform { float x, y; };
struct Velocity { float dx, dy; };

extern "C" {

SANDBOX_DECLARE_MODULE(movement_system)

int32_t movement_system_activate(flecs::world& ecs, const module_info* info) {
    
    ecs.component<Transform>();
    ecs.component<Velocity>();

    // Define a system that updates Transforms based on Velocities
    ecs.system<Transform, const Velocity>("MovementSystem")
        .each([](flecs::entity e, Transform& t, const Velocity& v) {
            t.x += v.dx;
            t.y += v.dy;
        });

    return 0;
}

}
```

You can now spawn entities in this plugin or any other plugin that requires `movement_system`!
