# 3.  Entity Component System (ECS) Design

The Sandbox Engine uses **Flecs** as its core Entity Component System. Because plugins interact with each other exclusively through the ECS and the Event Bus, strict architectural rules apply to how components are designed.

## The Rule of Plain Old Data (POD)
Flecs stores components contiguously in memory for extreme cache-locality performance. Because plugins can be written by different authors with different compiler settings, **components must be strictly flat, Plain Old Data (POD) structs.**

**Forbidden Component Patterns:**
```cpp
// DO NOT DO THIS
struct BadComponent {
    std::string name; // C++ object, size and layout varies by compiler
    std::vector<int> data; // Heap-allocated, destructor required
    virtual ~BadComponent() {} // vtable injection breaks C-ABI
};
```

**Allowed Component Patterns:**
```cpp
// DO THIS
struct GoodComponent {
    float x, y, z;
    uint32_t handle_id;
    bool is_active;
    char name[32]; // Fixed-size char array instead of std::string
};
```

## The "Handle" Pattern
If components must be plain old data, how do you manage heavy, dynamic C++ objects (like a Vulkan texture, an open file stream, or a complex AI state machine)?

You use the **Handle Pattern**:
1.  **Keep the Object Private:** The plugin allocates the heavy C++ object on its own private heap (e.g., using `std::unique_ptr` inside an internal array or registry managed by the plugin).
2.  **Pass a Handle:** The plugin generates a `uint32_t` or `uint64_t` Handle (an ID).
3.  **Store Handle in ECS:** The ECS component simply stores the handle ID.
4.  **Resolve locally:** When the plugin's Flecs `system` executes, it reads the `handle_id` from the component and looks up the real C++ object in its private array.

This ensures heavy C++ types never cross the ABI boundary, and the ECS memory footprint remains trivially copyable and blazingly fast.

## Singletons as Service Locators
Flecs provides a unique feature where components can be attached to the ECS world itself, essentially creating a Singleton.

The Sandbox Engine heavily exploits this to pass the pure C-ABI function wrappers to plugins.
For example, when the core `logger` module boots, it registers the `logger_service` struct as an ECS Singleton:
```cpp
ecs.set<sandbox::logger_service>(my_c_api_struct);
```

When a downstream plugin needs to log a message, it doesn't #include the logger implementation. It simply asks Flecs for the singleton pointer:
```cpp
// Correct API retrieval pattern using try_get / try_get_mut
if (auto* log_api = ecs.try_get<sandbox::logger_service>()) {
    log_api->log(log_api->instance, ...);
}
```

### Warning: `get` vs `try_get`
Modern versions of Flecs return a proxy reference when using `ecs.get<T>()`. This decays into complex reference types that compiler toolchains often fail to agree upon when compiling external DLLs. 
**Always use pointer-explicit methods:**
*   `ecs.try_get<T>()` for read-only access (returns `const T*`).
*   `ecs.try_get_mut<T>()` for mutable access (returns `T*`).
