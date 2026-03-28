# DIAGNOSTICS Extensions

## `diagnostics` (`sandbox/include/sandbox/diagnostics/diagnostics_ext.h`)

### Concept & Purpose
Collects profiling-oriented performance metrics (per-system timings and FPS) so runtime health can be inspected and reported consistently.

### State & Tags
- `metric`: `{ float last_ms, float avg_ms }`
- `state`: `{ std::unordered_map<std::string, metric> system_times, float fps }`
- `profile_tag`: marker component for entities that should be profiled.

### Events & Commands
From `sandbox/include/sandbox/diagnostics/diagnostics_evt.h`:
- No interaction structs are currently defined.

### Usage Example
```cpp
// Mark a system entity for profiling collection.
world.lookup("::systems::clock_tick").add<sandbox::extensions::diagnostics::profile_tag>();
```

## `logger` (`sandbox/include/sandbox/diagnostics/logger_ext.h`)

### Concept & Purpose
Provides extension-managed logging backed by spdlog and wired into the ECS event stream for low-coupling diagnostics emission.

### State & Tags
- Internal state: `std::unique_ptr<sandbox::logger> _logger`.
- No Flecs tag components are declared in `logger_ext.h`.

### Events & Commands
From `sandbox/include/sandbox/diagnostics/logger_evt.h`:
- `logger_trace_evt`: requires `message`.
- `logger_debug_evt`: requires `message`.
- `logger_info_evt`: requires `message`.
- `logger_warn_evt`: requires `message`.
- `logger_error_evt`: requires `message`.
- `logger_critical_evt`: requires `message`.

### Usage Example
```cpp
auto* ext_events = app.get_extension<sandbox::extensions::events>("events");
if (ext_events) {
    ext_events->publish(sandbox::diagnostics::logger_info_evt::create("Subsystem initialized"));
}
```
