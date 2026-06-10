# Sandbox Meta-Engine: Plugin Developer Guide

Welcome to the definitive guide for writing plugins for the Sandbox Meta-Engine! The engine's uncompromising C-ABI boundary means you never have to worry about mismatched STL implementations, compiler flag collisions, or memory corruption when your plugin crosses the dynamic library barrier.

## Architecture Deep Dive: The ABI Wall & The SDK Illusion

### Why the ABI Wall Exists
When dynamic plugins (`.dll` or `.so`) pass C++ Standard Library objects (like `std::string` or `std::vector`) to an engine, mismatched compiler versions or standard library implementations can lead to catastrophic memory boundary crashes.

The Sandbox Engine solves this by using the **Hourglass Pattern**. At the boundary bottleneck (the ABI Wall), everything is reduced to pure C-ABI primitives: raw byte arrays (`uint8_t*`), simple structs, and function pointers. 

### The SDK Illusion
You, as the plugin developer, don't need to manually serialize data into byte streams! The engine provides a robust **SDK Wrapper** layer. Under the hood, these wrappers (e.g., `sandbox::sdk::filesystem`) use **FlatBuffers** to safely serialize your data into the ABI format, beam it across the DLL boundary to the engine, retrieve the payload, and effortlessly unpack it back into beautiful `std::expected<std::vector<std::byte>, std::string>` objects. You get the safety of C-ABI and the elegance of modern C++23.

---

## How to Write a Plugin

### 1. The Entry Point
Every plugin must export a single entry point using the `SANDBOX_DECLARE_LIBRARY()` macro. The engine's Bootstrapper intercepts this and prepares the plugin to declare its modules inside the shared `flecs::world`.

### 2. Defining a Module
Use `SANDBOX_DECLARE_MODULE` to register your plugin's functionality into the Topological Bootstrapper. You must specify your module's name, version, and any service/module dependencies. The Bootstrapper guarantees your plugin won't boot until its required dependencies are fully instantiated!

```cpp
#include "sandbox/core/plugin.h"

struct my_plugin_module {
    my_plugin_module(flecs::world& ecs) {
        // Initialization logic here
    }
};

// SANDBOX_DECLARE_MODULE(Class, Name, Major, Minor, Patch, Service, Requirements...)
SANDBOX_DECLARE_MODULE(my_plugin_module, my_plugin, 1, 0, 0, "",
    {sandbox::requirement::kind::service, sandbox::requirement::strictness::require, "core_logger", 1, 0}
);

SANDBOX_DECLARE_LIBRARY()
```

---

## Using the APIs

Because the engine utilizes **Flecs** as the central dependency injector, you can retrieve the underlying subsystem API wrappers by querying the `flecs::world`.

```cpp
#include "sandbox/api/logger_api.h"
#include "sandbox/api/filesystem_api.h"
#include "sandbox/api/runner_api.h"

struct my_plugin_module {
    my_plugin_module(flecs::world& ecs) {
        // Safely retrieve the SDK wrappers!
        sandbox::sdk::logger logger(ecs);
        sandbox::sdk::filesystem vfs(ecs);
        sandbox::sdk::runner runner(ecs);

        // Log a beautiful message safely across the ABI
        logger.log(2, "My plugin has successfully booted!");

        // Read a file natively returning C++23 std::expected!
        auto text_result = vfs.read_text("mount://config.json");
        if (text_result) {
            logger.log(2, "Config Loaded: " + *text_result);
        }
    }
};
```

---

## The Event Bus

The Sandbox Meta-Engine leverages the Flecs Event pipeline to dispatch completely decoupled messages across the entire engine architecture.

```cpp
#include "sandbox/event_bus/event_bus.h"

// Define a simple event payload
struct my_custom_event {
    int secret_code;
};

struct my_plugin_module {
    my_plugin_module(flecs::world& ecs) {
        // Subscribe to an event
        sandbox::events::subscribe<my_custom_event>(ecs, [](const my_custom_event& ev) {
            // Handle the event securely!
        });

        // Publish an event to the rest of the engine synchronously
        sandbox::events::publish(ecs, my_custom_event{42});
    }
};
```

By adhering to these principles, your plugins will be safe, decoupled, and highly performant!
