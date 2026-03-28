# ECS Extensions

## `events` (`sandbox/include/sandbox/ecs/events_ext.h`)

### Concept & Purpose
Provides immediate event publishing and named observer management over Flecs, so subsystems communicate through typed payloads instead of direct hard references.

### State & Tags
- `transient_event_tag`: applied to per-publish transient source entities used for event emission and cleanup.

### Events & Commands
From `sandbox/include/sandbox/ecs/events_evt.h`:
- No interaction structs are currently defined.

### Usage Example
```cpp
auto* ext_events = app.get_extension<sandbox::extensions::events>("events");
if (ext_events) {
    bool exists = false;
    ext_events->publish(sandbox::ecs::system_exists_evt::create("clock_tick", &exists));
}
```

## `systems` (`sandbox/include/sandbox/ecs/systems_ext.h`)

### Concept & Purpose
Encapsulates creation and lifecycle management of Flecs systems with stable namespacing (`::systems::...`) and event-driven control hooks.

### State & Tags
- No persistent data structs are declared in `systems_ext.h`.
- Systems are represented as named Flecs entities under `::systems::`.

### Events & Commands
From `sandbox/include/sandbox/ecs/systems_evt.h`:
- `create_system_evt<components...>`: requires `name`, `stage`, and `action` callback.
- `destroy_system_evt`: requires `name`.
- `enable_system_evt`: requires `name`.
- `disable_system_evt`: requires `name`.
- `system_exists_evt`: requires `name` and `bool* out_exists`.
- `system_enabled_evt`: requires `name` and `bool* out_enabled`.

### Usage Example
```cpp
auto* ext_events = app.get_extension<sandbox::extensions::events>("events");
if (ext_events) {
    ext_events->publish(sandbox::ecs::disable_system_evt::create("serializer_save"));
}
```

## `triggers` (`sandbox/include/sandbox/ecs/triggers_ext.h`)

### Concept & Purpose
Wraps Flecs observers/triggers into named, controllable extension entities so reactive logic can be orchestrated with the same lifecycle model as systems.

### State & Tags
- No persistent data structs are declared in `triggers_ext.h`.
- Triggers are represented as named Flecs entities under `::triggers::`.

### Events & Commands
From `sandbox/include/sandbox/ecs/triggers_evt.h`:
- `create_trigger_evt<components...>`: requires `name` and `action` callback.
- `destroy_trigger_evt`: requires `name`.
- `enable_trigger_evt`: requires `name`.
- `disable_trigger_evt`: requires `name`.
- `trigger_exists_evt`: requires `name` and `bool* out_exists`.
- `trigger_enabled_evt`: requires `name` and `bool* out_enabled`.

### Usage Example
```cpp
auto* ext_events = app.get_extension<sandbox::extensions::events>("events");
if (ext_events) {
    bool enabled = false;
    ext_events->publish(sandbox::ecs::trigger_enabled_evt::create("cache_flag_removed", &enabled));
}
```

## `stages` (`sandbox/include/sandbox/ecs/stages_ext.h`)

### Concept & Purpose
Defines custom execution phases (`flecs::Phase`) with explicit dependency edges, enabling deterministic ordering across engine systems.

### State & Tags
- No persistent data structs are declared in `stages_ext.h`.
- Stages are represented as named Flecs entities under `::stages::`.

### Events & Commands
From `sandbox/include/sandbox/ecs/stages_evt.h`:
- `create_stage_evt`: requires `name`, optional `std::vector<std::string> dependencies`.
- `destroy_stage_evt`: requires `name`.
- `enable_stage_evt`: requires `name`.
- `disable_stage_evt`: requires `name`.
- `stage_exists_evt`: requires `name` and `bool* out_exists`.
- `stage_enabled_evt`: requires `name` and `bool* out_enabled`.

### Usage Example
```cpp
auto* ext_events = app.get_extension<sandbox::extensions::events>("events");
if (ext_events) {
    ext_events->publish(sandbox::ecs::create_stage_evt::create("Gameplay", {"PreUpdate"}));
}
```

## `scopes` (`sandbox/include/sandbox/ecs/scopes_ext.h`)

### Concept & Purpose
Provides controlled scope manipulation for hierarchical Flecs naming, so systems can compose entities in isolated namespaces.

### State & Tags
- No persistent data structs are declared in `scopes_ext.h`.
- Scope state is represented by Flecs world scope changes.

### Events & Commands
From `sandbox/include/sandbox/ecs/scopes_evt.h`:
- `set_scope_evt`: requires `std::vector<std::string> path`.
- `push_scope_evt`: requires `std::vector<std::string> path`.

### Usage Example
```cpp
auto* ext_events = app.get_extension<sandbox::extensions::events>("events");
if (ext_events) {
    ext_events->publish(sandbox::ecs::set_scope_evt::create({"extensions", "ai"}));
}
```
