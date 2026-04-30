# Core Architecture

This guide explains the foundational rules of the Sandbox framework: how memory and object lifetimes are managed, how extension types are registered, and how the engine resolves and initialises its dependency graph at startup.

---

## The Plugin Model

Every engine extension is a **plugin** — a heap-allocated object whose lifetime is owned by the `engine`.  
Each plugin holds a non-owning reference back to the `engine` that created it, so it can reach peer plugins and the shared ECS world.

```cpp
// sandbox/core/plugin.h (abridged)
namespace sandbox {
    class plugin {
        friend class engine;
    public:
        plugin(engine& eng);       // injected reference to the owning engine
        virtual ~plugin();

        engine& context;           // non-owning back-reference

    private:
        virtual void initialize() = 0;  // called by engine after construction
        virtual void finalize()   = 0;  // called by engine before destruction
    };
}
```

`initialize()` and `finalize()` are **private pure virtual** methods.  
Making them private is intentional: only the `engine` (declared `friend`) may call them, which enforces that the lifecycle is always driven by the framework rather than by the plugin itself.

---

## Defining a New Extension

### 1 — Inherit from `plugin`

```cpp
#include "sandbox/core/plugin.h"

namespace my_game {
    class audio_plugin : public sandbox::plugin {
    public:
        explicit audio_plugin(sandbox::engine& ctx)
            : sandbox::plugin(ctx) {}

    private:
        void initialize() override {
            // acquire resources, subscribe to events, …
        }

        void finalize() override {
            // release resources, unsubscribe, …
        }
    };
}
```

### 2 — Register the type with the reflection system

The engine resolves plugin types **by name at runtime** using RTTR.  
A plugin class that is never registered cannot be loaded from a manifest.

Use the `SANDBOX_REFLECTION` macro (a thin wrapper around `RTTR_REGISTRATION`) in a dedicated `.cpp` translation unit:

```cpp
// audio_plugin_registration.cpp
#include "sandbox/core/type_registration.h"
#include "my_game/audio_plugin.h"

SANDBOX_REFLECTION {
    SANDBOX_REGISTER_TYPE(my_game::audio_plugin)
}
```

`SANDBOX_REGISTER_TYPE(T)` registers the class under the string `"T"` (its fully-qualified C++ name).  
Use `SANDBOX_REGISTER_TYPE_NAMED(T, "custom_name")` when you need a shorter or version-stable alias:

```cpp
SANDBOX_REFLECTION {
    SANDBOX_REGISTER_TYPE_NAMED(my_game::audio_plugin, "audio")
}
```

> **Important:** the `SANDBOX_REFLECTION` block must be compiled and linked into the final binary.  
> Placing it in a static library that nothing else references can cause the linker to strip it.  
> Prefer adding registrations to a translation unit that is part of the executable, or force-link the object file.

---

## The Manifest and Initialisation Flow

The engine is bootstrapped by calling `engine::initialize(manifest)` with a `sandbox::properties` object that carries a JSON description of which plugins to load.

### Manifest schema

```json
{
  "plugins": {
    "<alias>": {
      "type": "<registered_type_name>"
    }
  }
}
```

`<alias>` is the name under which the plugin instance is stored in the ECS world and later retrieved via `engine::get_plugin` / `engine::find_plugin`.  
`<registered_type_name>` must exactly match the name used in `SANDBOX_REGISTER_TYPE` or `SANDBOX_REGISTER_TYPE_NAMED`.

### Loading a manifest from code

```cpp
sandbox::properties manifest;
manifest.load_from_file("config/engine.json");

sandbox::engine engine;
engine.initialize(manifest);
```

### Loading a manifest inline

```cpp
sandbox::properties manifest;
manifest.set({"plugins", "audio",   "type"}, "audio");
manifest.set({"plugins", "physics", "type"}, "my_game::physics_plugin");

sandbox::engine engine;
engine.initialize(manifest);
```

### What `engine::initialize` does internally

```
for each alias in manifest["plugins"]:
    1. read manifest["plugins"][alias]["type"]
    2. call type_registry::instantiate<plugin>(type_name, this)  // 'this' = the engine instance
          → RTTR looks up the registered constructor by name
          → constructs the plugin with `new`, returns std::unique_ptr<plugin>
    3. store the unique_ptr as a component on an ECS entity named `alias`
          (entity is created under the "::plugins" scope)
```

If `type` is missing for any entry, `engine::initialize` throws `std::runtime_error`.

---

## Dependency Resolution Graph

The framework uses a **manifest-ordering strategy**: dependencies are resolved by placing a plugin's dependencies **earlier** in the `"plugins"` object than the plugin that needs them.

### The rule

> A plugin may safely call `context.find_plugin<T>("alias")` from its `initialize()` method **only for plugins that appear before it** in the manifest, because those plugins are guaranteed to have been constructed and stored in the ECS by the time `initialize()` is called on later plugins.

### Conceptual graph

Given the manifest:

```json
{
  "plugins": {
    "asset_cache":  { "type": "my_game::asset_cache_plugin"  },
    "audio":        { "type": "my_game::audio_plugin"        },
    "renderer":     { "type": "my_game::renderer_plugin"     }
  }
}
```

The implied dependency graph is:

```
asset_cache  ──►  (no dependencies)
audio        ──►  asset_cache
renderer     ──►  asset_cache, audio
```

Initialisation order follows declaration order: `asset_cache` → `audio` → `renderer`.

### Accessing a dependency at runtime

```cpp
void my_game::audio_plugin::initialize() {
    // "asset_cache" was declared before "audio" in the manifest,
    // so it is guaranteed to exist here.
    auto* cache = context.find_plugin<my_game::asset_cache_plugin>("asset_cache");
    if (!cache) {
        throw std::runtime_error("audio_plugin requires asset_cache_plugin");
    }
    // use cache …
}
```

`engine::find_plugin<T>` performs a `dynamic_cast` from the stored `plugin*`, so the cast will return `nullptr` if the alias refers to a plugin of an incompatible type.

### What the graph does NOT do automatically

- **Cycle detection** — declaring two plugins that each depend on the other is not caught at load time; you must avoid cycles in the manifest.
- **Topological sort** — the engine does not reorder entries; the author is responsible for correct ordering in the manifest.
- **Late binding** — a plugin cannot safely access a peer that was declared *after* it in the manifest during `initialize()`; those plugins do not yet exist.

---

## Accessing the ECS World

Every plugin receives a reference to the `engine`, which exposes the shared `flecs::world` as `engine::ecs`.  
Within `initialize()` and `finalize()`, plugins can create entities, register components, and add systems directly:

```cpp
void my_game::audio_plugin::initialize() {
    context.ecs.system<AudioSource, Transform>()
        .each([](AudioSource& src, const Transform& xf) {
            // update spatial audio …
        });
}
```

All entities created inside `initialize()` while a `SANDBOX_SCOPE_GUARD` is active are automatically parented to the scope entity, which keeps the ECS hierarchy tidy and supports clean teardown.

---

## Summary

| Concept | Mechanism |
|---------|-----------|
| Define an extension | Inherit `sandbox::plugin`, override `initialize()` / `finalize()` |
| Register a type | `SANDBOX_REFLECTION { SANDBOX_REGISTER_TYPE(MyPlugin) }` in a compiled `.cpp` |
| Load extensions | `engine::initialize(manifest)` reads the `"plugins"` manifest section |
| Declare dependencies | List dependents **after** their dependencies in the manifest |
| Resolve a dependency at runtime | `context.find_plugin<T>("alias")` inside `initialize()` |
| Access the ECS world | `context.ecs` (a `flecs::world`) |
