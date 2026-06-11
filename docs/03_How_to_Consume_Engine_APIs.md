# 3. How to Consume Engine APIs

*This practical guide shows you exactly how to fetch and use the internal engine services provided as Flecs Singletons.*

**Conclusion first:** To consume an API, you must request the raw C-ABI `_service` pointer from the ECS using `ecs.try_get<T>()`, wrap it in the corresponding C++23 `sdk::` class, and use the `std::expected` results.

## The C++ Pattern for Fetching APIs
Because the Sandbox Engine avoids global state across DLLs, all core services are registered as singletons inside the Flecs `world`.

**Warning:** Never use `ecs.get<T>()`! It returns a Flecs proxy reference that modern compilers struggle to resolve correctly across DLL boundaries. Always use the explicit pointer methods: `ecs.try_get<T>()` (read-only) or `ecs.try_get_mut<T>()` (mutable).

### Example: Consuming the Filesystem Service

```cpp
#include <sandbox/api/filesystem_api.h>
#include <sandbox/api/logger_api.h>

void load_player_data(flecs::world& ecs) {
    // 1. Fetch the raw C-ABI function pointer struct
    const auto* fs_api = ecs.try_get<sandbox::filesystem_service>();
    
    if (!fs_api) {
        SANDBOX_ERROR(ecs, "Filesystem API is not loaded!");
        return;
    }

    // 2. Wrap it in the C++ SDK
    sandbox::sdk::filesystem fs(fs_api);

    // 3. Use the SDK. It returns std::expected (C++23)
    auto file_result = fs.read_text("mount://cache/player.json");

    if (file_result.has_value()) {
        SANDBOX_INFO(ecs, "Loaded player data: {}", file_result.value());
    } else {
        SANDBOX_WARN(ecs, "Failed to load player data: {}", file_result.error());
    }
}
```

### Advanced: Using the Convenience Wrappers
The engine provides inline convenience wrappers in the `sandbox::api` namespace that automatically perform the `try_get` and `sdk` wrapping for you if you don't want to manage the object yourself:

```cpp
// This does the exact same thing as the example above, but in one line!
auto file_result = sandbox::api::read_text(ecs, "mount://cache/player.json");
```
