# IO Extensions

## `filesystem` (`sandbox/include/sandbox/io/filesystem_ext.h`)

### Concept & Purpose
Provides virtual-to-physical path resolution through mount tables, allowing engine code to target stable virtual prefixes instead of platform-dependent absolute paths.

### State & Tags
- `state`: `std::unordered_map<std::string, std::string> mounts`.
- `file_changed_event`: event payload containing `virtual_path` and `physical_path`.
- `read_request`: request component with `path`.

### Events & Commands
From `sandbox/include/sandbox/io/filesystem_evt.h`:
- `resolve_path_evt`: requires `path` and `std::string* out_result`.

### Usage Example
```cpp
auto* ext_events = app.get_extension<sandbox::extensions::events>("events");
if (ext_events) {
    std::string resolved;
    ext_events->publish(sandbox::io::resolve_path_evt::create("res://textures/ui.png", &resolved));
}
```

## `serializer` (`sandbox/include/sandbox/io/serializer_ext.h`)

### Concept & Purpose
Coordinates persistence workflows by reacting to save/load request components and transforming entity component state to and from JSON.

### State & Tags
- `config`: `{ std::string root_dir }`
- `save_request`: request component with `filename`.
- `load_request`: request component with `filename`.
- `persistent_tag`: marker for entities participating in persistence workflows.

### Events & Commands
From `sandbox/include/sandbox/io/serializer_evt.h`:
- No interaction structs are currently defined.

### Usage Example
```cpp
// Trigger save workflow by adding a request component to an entity.
auto save_target = world.entity("profile_entity");
save_target.set<sandbox::extensions::serializer::save_request>({"profiles/player_01.json"});
```
