# DATA Extensions

## `caches` (`sandbox/include/sandbox/data/caches_ext.h`)

### Concept & Purpose
Adds cache-aside lookup acceleration for named ECS entities and coordinated cleanup/expiration logic for runtime-managed handles.

### State & Tags
- `cache_data`: `{ flecs::entity entity, float lifetime, float last_access_time }`
- `_caches`: `std::unordered_map<std::string, cache_data>` internal state.
- `cache_flag`: tag attached to cached entities.

### Events & Commands
From `sandbox/include/sandbox/data/caches_evt.h`:
- `save_cache_evt`: requires `flecs::entity entity`, optional `float lifetime`.
- `free_cache_evt`: requires `name`.
- `get_cache_evt`: requires `name` and `flecs::entity* out_entity`.

### Usage Example
```cpp
auto* ext_events = app.get_extension<sandbox::extensions::events>("events");
if (ext_events) {
    flecs::entity cached;
    ext_events->publish(sandbox::data::get_cache_evt::create("events", &cached));
}
```

## `storage` (`sandbox/include/sandbox/data/storage_ext.h`)

### Concept & Purpose
Implements typed object lifecycle management on ECS entities under `::objects::...`, enabling world-backed object construction, lookup, and destruction.

### State & Tags
- No dedicated persistent state struct is declared in `storage_ext.h`.
- Stored objects live as typed components on named entities (`::objects::<name>`).

### Events & Commands
From `sandbox/include/sandbox/data/storage_evt.h`:
- `create_object_evt<base_type>`: requires `name` and `action` callback.
- `destroy_object_evt`: requires `name`.
- `get_object_evt<base_type>`: requires `name`, `base_type** out_object`, and `action` callback.
- `object_exists_evt`: requires `name` and `bool* out_exists`.

### Usage Example
```cpp
auto* ext_events = app.get_extension<sandbox::extensions::events>("events");
if (ext_events) {
    bool exists = false;
    ext_events->publish(sandbox::data::object_exists_evt::create("main_camera", &exists));
}
```
