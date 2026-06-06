# Sandbox Engine — Developer Wiki

**Version:** 1.0 | **Standard:** C++23 | **License:** MIT

This document is the exhaustive technical reference for the Sandbox Meta-Engine. It is structured for developers integrating with the engine, building plugins, or extending the core subsystems.

---

## Table of Contents

- [I. Architectural Foundation](#i-architectural-foundation)
  - [1.1 Design Philosophy](#11-design-philosophy)
  - [1.2 Managing Dependencies](#12-managing-dependencies)
  - [1.3 ABI & Plugin Safety](#13-abi--plugin-safety)
- [II. Systems Reference](#ii-systems-reference)
  - [2.1 Filesystem Subsystem](#21-filesystem-subsystem)
  - [2.2 Logger Subsystem](#22-logger-subsystem)
  - [2.3 Runner Subsystem](#23-runner-subsystem)
  - [2.4 Properties Utility](#24-properties-utility)
- [III. Developer's Handbook](#iii-developers-handbook)
  - [3.1 Creating a Plugin](#31-creating-a-plugin)
  - [3.2 Declaring Modules](#32-declaring-modules)
  - [3.3 Declaring Services](#33-declaring-services)
  - [3.4 Manifest Reference](#34-manifest-reference)
  - [3.5 Engine Environment Configuration](#35-engine-environment-configuration)
- [IV. Technical Specifications](#iv-technical-specifications)
  - [4.1 Threading Model](#41-threading-model)
  - [4.2 VFS Specification](#42-vfs-specification)
  - [4.3 Event Bus Architecture](#43-event-bus-architecture)
  - [4.4 Logging Macros Reference](#44-logging-macros-reference)

---

# I. Architectural Foundation

## 1.1 Design Philosophy

Sandbox is built on two foundational principles: **Data-Driven Configuration** and **Plugin-Oriented Architecture**.

**Data-Driven Configuration** means that no engine behavior is hard-coded into the runtime binary. What modules to load, at what FPS to run, where to write cache files — all of this is determined at runtime from configuration data. The engine binary itself is agnostic to the application it runs.

**Plugin-Oriented Architecture** means that every application-level feature is a separate shared library (`.dll` / `.so` / `.dylib`). The engine binary contains only the core subsystems (Logger, Filesystem, Runner) and the infrastructure to load, link, and orchestrate plugins. The application's modules — whether they are renderers, data processors, web servers, or ML trainers — are plugins that the engine discovers and imports at runtime.

The consequence of this design is strong **Separation of Concerns**:
- The engine does not know what a renderer or web server is.
- A processing module does not need to include engine headers to access a logger.
- Modules communicate entirely through **typed events** on the **ECS world** and **Service components**.

### Key Terminology

| Term | Definition |
|---|---|
| **Engine** | The `sandbox::engine` class. Owns the ECS world and orchestrates startup/shutdown. |
| **Module** | A C++ struct that registers itself with Flecs via `ecs.import<T>()`. The unit of functionality. |
| **Plugin** | A shared library (`.dll` / `.so` / `.dylib`) that packages one or more Modules and exports `SandboxLibraryMain`. |
| **Service** | A named interface pointer stored as a singleton ECS component. The bridge between modules. |
| **Manifest** | A `manifest.json` file inside the application archive declaring which top-level Modules to activate. |
| **Mount** | A virtual path prefix (`mount://app`, `mount://cache`, `mount://bin`) that maps to a physical path or archive. |
| **Bootstrapper** | The engine component that resolves the Module dependency graph and imports Modules in order. |
| **Environment** | The `engine_environment` ECS entity (`::Sandbox::Environment`) holding the runtime config map. |

---

## 1.2 Managing Dependencies

The engine handles plugin loading order automatically. As a plugin developer, you do not need to worry about the internal boot process. Instead, you declare what your module needs using the `SANDBOX_DECLARE_MODULE` macro.

### Using `sandbox::requirement`

When declaring a module, you provide a list of requirements. The engine guarantees that all required targets (either specific Modules or broad Services) are fully loaded before your module is instantiated.

**Example: Module A requires Service B**

```cpp
// This module requires a logger_service before it can boot.
// The engine will ensure that whatever plugin provides "logger_service"
// is loaded and initialized first.
SANDBOX_DECLARE_MODULE(
    my_custom_module, my_module_name, 1, 0, "",
    {sandbox::requirement::kind::service, sandbox::requirement::strictness::require, "logger_service", 1, 0}
);
```

If the required service or module cannot be found in any of the available plugins, the engine will safely abort the boot process and report the missing dependency.

---

## 1.3 ABI & Plugin Safety

A Plugin is a standard platform shared library that exports a single C-linkage function:

```c
extern "C" SANDBOX_EXPORT void SandboxLibraryMain(ecs_world_t* world);
```

Flecs uses the platform's dynamic loading API (`dlopen`/`LoadLibrary`) to load the shared library (`.dll` / `.so` / `.dylib`). The engine uses `SANDBOX_DECLARE_LIBRARY()` to generate this entry point automatically.

**ABI contract:** Plugin shared libraries are loaded in-process. They share the same Flecs world pointer. The plugin must be compiled against the same Flecs version and the same `sandbox/core/*.h` public headers as the engine. The engine exports no complex third-party template types across the DLL boundary; Services are raw interface pointers.

---

# II. Systems Reference

## 2.1 Filesystem Subsystem

The Filesystem maps physical paths to virtual `mount://` prefixes. This prevents plugins from relying on hardcoded system paths and natively supports reading directly from archived packages (e.g. `.zip`).

**Example: Reading a file through the Event Bus**
```cpp
auto data = SANDBOX_FS_EXEC_READ(ecs, "mount://app/config.json");
```

## 2.2 Logger Subsystem

The Logger subsystem provides a structured, level-gated logging interface backed by spdlog. All engine-internal logging and plugin logging goes through the ECS event macros, ensuring messages are uniformly captured regardless of which shared library emitted them.

**Example: Emitting logs**
```cpp
SANDBOX_INFO(ecs, "[MyPlugin] Initialization complete. Loaded {} assets.", count);
SANDBOX_ERROR(ecs, "[MyPlugin] Failed to locate required data!");
```

## 2.3 Runner Subsystem

The Runner subsystem manages the engine's main execution loop. It can block the main thread or spawn an asynchronous worker thread to progress the Flecs ECS pipeline at a targeted framerate.

**Example: Requesting a state change**
```cpp
// Request the engine to pause
sandbox::events::publish(ecs, sandbox::events::runner::state_change{
    sandbox::events::runner::state_change::action::Pause
});
```

## 2.4 Properties Utility

`sandbox::properties` is a JSON document wrapper backed by Glaze. It provides a typed, path-based API for structured configuration data. The engine stores the application Manifest as a `properties` component on the `::Manifest` ECS entity.

**Example: Reading data from the Manifest**
```cpp
auto& manifest = ecs.entity("::Manifest").get<sandbox::properties>();
auto timeout = manifest.get<int>({"config", "network", "timeout_ms"});
int ms = timeout.value_or(5000);
```

---

# III. Developer's Handbook

## 3.1 Creating a Plugin

A Plugin is a shared library (`.dll` / `.so` / `.dylib`) that packages one or more Modules. The complete structure is:

```
myplugin/
├── CMakeLists.txt
└── source/
    └── myplugin.cpp
```

**`CMakeLists.txt`:**
```cmake
add_library(myplugin MODULE source/myplugin.cpp)

target_link_libraries(myplugin PRIVATE sandbox)
target_include_directories(myplugin PRIVATE ${sandbox_INCLUDE_DIRS})

# Place the shared library in the modules directory so the engine can discover it
set_target_properties(myplugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/modules"
    PREFIX ""
)
```

**`source/myplugin.cpp` — Minimal "Hello World" Plugin:**
```cpp
#include "sandbox/core/plugin.h"   // SANDBOX_DECLARE_MODULE, SANDBOX_DECLARE_LIBRARY
#include "sandbox/core/service.h"  // SANDBOX_DECLARE_SERVICE

// ============================================================================
// Step 1: Declare any Services this Plugin produces or consumes
// ============================================================================

struct i_my_interface {};  // The abstract interface struct

SANDBOX_DECLARE_SERVICE(my_service, i_my_interface)

// ============================================================================
// Step 2: Define your Module
// ============================================================================

struct hello_module {
    hello_module(flecs::world& ecs) {
        // Register this module as the provider of my_service
        ecs.set<my_service>({nullptr, 1, 0});

        // Use the ECS-routed logger
        // (requires logger_service to already be loaded — declare it as a requirement below)
        SANDBOX_INFO(ecs, "[Hello] Module booted.");
    }
};

// ============================================================================
// Step 3: Register the Module
// ============================================================================

SANDBOX_DECLARE_MODULE(
    hello_module,              // C++ struct name
    hello_world,               // Module name (used in manifest.json and requirements)
    1,                         // Version major
    0,                         // Version minor
    "my_service",              // Service this Module provides (empty string if none)
    // Requirements:
    {sandbox::requirement::kind::service, sandbox::requirement::strictness::require, "logger_service", 1, 0}
);

// ============================================================================
// Step 4: Export the Plugin entry point
// ============================================================================

SANDBOX_DECLARE_LIBRARY()
```

## 3.2 Declaring Modules

`SANDBOX_DECLARE_MODULE` generates a static inline initializer that registers a `module_info` descriptor.

```cpp
#define SANDBOX_DECLARE_MODULE(Class, Name, Major, Minor, Service, ...)
```

| Parameter | Type | Description |
|---|---|---|
| `Class` | C++ struct | The Module struct. Must have a constructor `(flecs::world& ecs)`. |
| `Name` | identifier | The Module's string name. Used in `manifest.json` and requirements. |
| `Major` | integer | Semantic version major. |
| `Minor` | integer | Semantic version minor. |
| `Service` | string literal | The Service name this Module provides. Use `""` if none. |
| `...` | `requirement{}` | Zero or more `sandbox::requirement` entries. |

## 3.3 Declaring Services

A **Service** is a singleton ECS component that exposes an abstract interface pointer. It is the canonical mechanism for inter-module communication.

```cpp
#define SANDBOX_DECLARE_SERVICE(service_name, interface_type)
// Expands to:
struct service_name {
    static constexpr const char* type_name = "service_name";
    interface_type* api{nullptr};
    uint8_t version_major{1};
    uint8_t version_minor{0};
};
```

## 3.4 Manifest Reference

The **Manifest** is a `manifest.json` file located at the root of the application archive (`mount://app/manifest.json`). The engine reads it during initialization and stores it as a `sandbox::properties` component on the `::Manifest` entity.

### Schema

```json
{
    "modules": [
        "module_name_1",
        "module_name_2"
    ]
}
```

The engine activates each module listed. You can also append custom application data to the manifest and read it later using the Properties utility.

## 3.5 Engine Environment Configuration

All engine and subsystem configuration is passed as a `std::unordered_map<std::string, std::any>`. This map is stored in the ECS world as `engine_environment` on the `::Sandbox::Environment` entity and is available to all subsystems.

### Reading Configuration in a Subsystem

Use `get_config<T>()` from `sandbox/utilities/config_helper.h`:

```cpp
#include "sandbox/utilities/config_helper.h"

auto env_entity = ecs.entity("::Sandbox::Environment");
std::unordered_map<std::string, std::any> config;
if (env_entity.has<engine_environment>()) {
    config = env_entity.get<engine_environment>().config;
}

// Safe extraction with type check and fallback
int fps = sandbox::get_config<int>(config, "fps_limit", 60);
```

---

# IV. Technical Specifications

## 4.1 Threading Model

The Runner subsystem executes the main engine loop and can operate in either synchronous or asynchronous mode. 

**Synchronous Mode** (`run_sync`) blocks the calling thread (e.g., the main application thread). The ECS tick loop executes sequentially on the thread that invoked it. This is the standard behavior for most dedicated applications.

**Asynchronous Mode** (`start_async`) spawns a dedicated background worker thread to process the ECS tick loop. This allows the host process to continue doing work on the main thread — such as processing OS window events, handling external UI frameworks, or interfacing with external APIs — while the logic ticks independently in the background.

## 4.2 VFS Specification

The Virtual Filesystem maps every file access through a unified `mount://` scheme. 

### Standard Mount Points

| Mount Prefix | Physical Location | Writable | Description |
|---|---|---|---|
| `mount://cache` | OS user data directory | ✅ Yes | Writable output: saved modules, save files, generated content |
| `mount://bin` | Executable directory | ❌ No | Engine binaries and bundled modules |
| `mount://app` | Application archive path | ❌ No | Application assets, manifest, packaged modules |

The user data directory is resolved by `sandbox::filesystem::get_user_data_directory()`, which returns the platform-appropriate location (e.g., `~/.local/share/sandbox/` on Linux).

## 4.3 Event Bus Architecture

The Event Bus (`sandbox/event_bus/event_bus.h`) provides a type-safe, ECS-integrated publish/subscribe system. Events are Flecs component types published to the ECS world.

```cpp
namespace sandbox::events {
    // Synchronous: published and handled immediately
    template<typename EventType>
    void publish(flecs::world world, const EventType& payload,
                 flecs::entity channel = flecs::entity());

    // Subscribe: returns a Flecs entity (the observer). Destroy it to unsubscribe.
    template<typename EventType, typename Func>
    flecs::entity subscribe(flecs::world world, Func&& callback,
                            flecs::entity channel = flecs::entity());
}
```

## 4.4 Logging Macros Reference

All macros defined in `sandbox/event_bus/logger_events.h`. The first argument is always a `flecs::world&`.

| Macro | Level | Debug-only |
|---|---|---|
| `SANDBOX_TRACE(ecs, fmt, ...)` | Trace | ✅ |
| `SANDBOX_DEBUG(ecs, fmt, ...)` | Debug | ✅ |
| `SANDBOX_INFO(ecs, fmt, ...)`  | Info  | ❌ |
| `SANDBOX_WARN(ecs, fmt, ...)`  | Warn  | ❌ |
| `SANDBOX_ERROR(ecs, fmt, ...)` | Error | ❌ |
| `SANDBOX_FATAL(ecs, fmt, ...)` | Fatal | ❌ |

All macros accept `std::format`-style format strings:
```cpp
SANDBOX_INFO(ecs, "[{}] Loaded {} entities in {:.2f}ms.", module_name, count, duration_ms);
```
