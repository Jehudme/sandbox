# 10. Memory Ownership Rules

*This explanation clarifies the strict rules surrounding heap allocations across the DLL boundary.*

## The Core Rule
**Memory must be freed by the same allocator that allocated it.**

If Plugin A allocates a `std::string` using `new`, and hands it to Plugin B, and Plugin B calls `delete`... the engine will crash. Windows and Linux maintain distinct heap allocators per DLL boundary if they are compiled statically against the runtime.

## Rule 1: The Plugin Owns Its Heap
When a plugin loads, it creates its own memory pool. If you create a heavy C++ object (like a Vulkan Context or an AI State Machine), you must store it in a `std::vector` or `std::unique_ptr` *inside your plugin's struct*.

Do not pass this object to the ECS or to other plugins! Instead, generate a `uint32_t` Handle (an integer ID), pass the ID to the ECS, and look the object up locally when your plugin executes.

## Rule 2: The Engine Owns the ECS
Flecs (the Entity Component System) is owned by the core engine executable. Plugins are handed a `flecs::world&` reference. 
You are permitted to call `ecs.component<T>()` or `ecs.system()`. The memory allocated for these components is managed by the central engine. 
When the engine shuts down, it cleans up the entire Flecs world, automatically sweeping away all entities and components your plugin created.

## Rule 3: Payload Callbacks
When an array of bytes needs to cross the DLL boundary (for example, a JSON configuration string or a serialized FlatBuffer event), we use the `sandbox_payload` struct.

```cpp
typedef struct sandbox_payload {
    uint8_t* bytes;
    size_t size;
    void (*free_func)(void*); // The magic bullet
} sandbox_payload;
```

If Plugin A calls a function in Plugin B to get a list of files, Plugin B `malloc`s a byte array. 
Plugin B puts the pointer in `bytes`, and assigns `free_func` to its own internal `free` implementation.
Plugin A reads the bytes, and when it is finished, Plugin A executes `payload.free_func(payload.bytes)`. 

This guarantees the memory is deallocated on Plugin B's heap, entirely dodging the cross-DLL heap corruption problem! The `sandbox::sdk::payload` C++ wrapper does this automatically in its destructor.
