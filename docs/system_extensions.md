# SYSTEM Extensions

## `clock` (`sandbox/include/sandbox/system/clock_ext.h`)

### Concept & Purpose
Maintains frame delta, fixed-step accumulation, and time scaling so simulation timing stays explicit and centralized.

### State & Tags
- `state`:
  - `dt`
  - `fixed_dt`
  - `total_time`
  - `time_scale`
  - `accumulator`
- `fixed_update_tag`: marker for fixed-step-driven logic.
- `time_scale_request`: `{ float target_scale, float lerp_speed }`.

### Events & Commands
From `sandbox/include/sandbox/system/clock_evt.h`:
- No interaction structs are currently defined.

### Usage Example
```cpp
// Request smooth time-scale transition on the world.
world.entity("clock_control").set<sandbox::extensions::clock::time_scale_request>({0.5f, 0.2f});
```

## `dependencies` (`sandbox/include/sandbox/system/dependencies_ext.h`)

### Concept & Purpose
Tracks required extensions and marks readiness when dependency contracts are satisfied, enabling startup gating for dependent systems.

### State & Tags
- `state`: `std::vector<std::string> requirements`.
- `is_ready_tag`: applied to the world when all required extensions are available.

### Events & Commands
From `sandbox/include/sandbox/system/dependencies_evt.h`:
- `require_dependency_evt`: requires `extension_name`.
- `validate_dependencies_evt`: no fields.

### Usage Example
```cpp
auto* ext_events = app.get_extension<sandbox::extensions::events>("events");
if (ext_events) {
    ext_events->publish(sandbox::system::require_dependency_evt::create("filesystem"));
    ext_events->publish(sandbox::system::validate_dependencies_evt::create());
}
```
