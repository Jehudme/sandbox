# Sandbox Wiki

## Writing a Module
Creating a module for Sandbox Engine uses standard `Flecs` ECS logic wrapped in our `SANDBOX_DECLARE_MODULE` macro to ensure ABI safety.

### 1. Simplified API Namespace
Instead of interacting with internal engine components and headers, use the zero-cost `sandbox::api` functions, which safely route data through C-ABI barriers using FlatBuffers.

```cpp
#include <sandbox/core/plugin.h>
#include <sandbox/api/logger_api.h>
#include <sandbox/api/filesystem_api.h>
#include <sandbox/api/runner_api.h>

struct my_module {
    my_module(flecs::world& ecs) {
        
        // Log a message safely using the engine's standardized macro
        SANDBOX_INFO(ecs, "My Module has started!");
        
        // Use the C++ SDK wrappers for easy, safe API interactions
        if (auto* fs_api = ecs.try_get<sandbox::filesystem_service>()) {
            sandbox::sdk::filesystem fs(fs_api);
            auto file = fs.read_text("mount://app/config.json");
            if (file) {
                SANDBOX_INFO(ecs, "Read config size: {}", file->size());
            } else {
                SANDBOX_WARN(ecs, "Failed to read config.json");
            }
        }
    }
};
```

### 2. Declaring and Registering
Use the `SANDBOX_DECLARE_MODULE` macro at the bottom of your file. This registers the module with Kahn's topological sort bootstrapper, allowing you to define precise version constraints.

```cpp
// Register my_module, named "cool_feature", version 1.0.0
// Declare dependencies on the engine's core services:
SANDBOX_DECLARE_MODULE(my_module, cool_feature, 1, 0, 0, "",
    {sandbox::requirement::kind::service, sandbox::requirement::strictness::require, "logger_service", 1, 0},
    {sandbox::requirement::kind::service, sandbox::requirement::strictness::require, "filesystem_service", 1, 0}
);

// Define the DLL Entry Point
SANDBOX_DECLARE_LIBRARY()
```

## The Engine as a Library (Static Linking)
You can optionally link your plugins *statically* directly into the Sandbox engine for easier debugging.
Because the engine uses the exact same `SANDBOX_DECLARE_MODULE` macros for its own internal systems (`core_logger`, `core_vfs`), your plugin will cleanly integrate into the same global registry and execute properly without requiring a dynamic DLL load step!

## Exception Safety & ABI Stability
We strictly enforce a "No C++ Exceptions Across the DLL Boundary" rule.
- The Engine boundaries use pure C-ABI structures with function pointers (e.g. `logger_service`, `filesystem_service`) to guarantee 100% stable compatibility across different compilers.
- If your module's `import_fn` constructor throws an exception, the engine catches it and correctly aborts the boot process without corrupting memory.
- C++ SDK wrappers (like `sandbox::sdk::filesystem`) map raw integer error codes to modern `std::expected` (C++23) rather than throwing an exception across ABI boundaries.

### Version Mapping in the Manifest
To load your module dynamically, ensure it exists in `manifest.json`:

```json
{
  "modules": {
    "cool_feature": "1.0",
    "another_plugin": "2.1"
  }
}
```
