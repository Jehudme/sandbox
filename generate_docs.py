import os

files = {
    "docs/README.md": """# Sandbox Engine Documentation

Welcome to the Sandbox Engine documentation. This documentation is structured using the [Diátaxis Framework](https://diataxis.fr/), dividing content into four distinct quadrants.

## 🧭 Table of Contents

### 1. Tutorials (Learning-Oriented)
*Lessons that take the reader by the hand through a series of steps to complete a project.*
- [Your First Plugin](tutorials/01_first_plugin.md)
- [ECS Systems & Components](tutorials/02_ecs_systems.md)

### 2. How-To Guides (Problem-Oriented)
*Directions that take the reader through the steps required to solve a real-world problem.*
- [How to Use the Event Bus](how-to/01_use_event_bus.md)
- [How to Read and Write to the Virtual File System](how-to/02_read_write_vfs.md)
- [How to Define FlatBuffer Schemas](how-to/03_define_flatbuffer_schemas.md)

### 3. Reference (Information-Oriented)
*Technical descriptions of the machinery and how to operate it.*
- [Manifest.json Reference](reference/01_manifest_json.md)
- [Launcher CLI Arguments](reference/02_launcher_args.md)
- [Core Services API](reference/03_core_services_api.md)
- [Supported VFS Formats](reference/04_vfs_formats.md)

### 4. Explanation (Understanding-Oriented)
*Discussions that clarify and illuminate a particular topic.*
- [The Meta-Engine Paradigm](explanation/01_meta_engine_paradigm.md)
- [The C-ABI Firewall](explanation/02_c_abi_wall.md)
- [Kahn's Sort & The Bootstrapper](explanation/03_kahn_sort_bootstrapper.md)
- [Memory Ownership & FlatBuffers](explanation/04_memory_ownership_flatbuffers.md)
""",

    "docs/tutorials/01_first_plugin.md": """# Tutorial: Your First Plugin

This tutorial will guide you through creating your first dynamic plugin for the Sandbox Meta-Engine.

## Prerequisites
- A C++23 compliant compiler.
- CMake 3.20 or higher.
- Understanding of the `SANDBOX_DECLARE_MODULE` macro.

## Step 1: Create the Source File

Create a file named `my_plugin.cpp`.

```cpp
#include <sandbox/core/plugin.h>
#include <sandbox/modules/logger/logger_api.h>
#include <iostream>

using namespace sandbox;

extern "C" {

SANDBOX_DECLARE_MODULE(my_plugin)

int32_t my_plugin_activate(flecs::world& ecs, const module_info* info) {
    // Obtain the logger SDK wrapper
    sdk::logger log(ecs);
    log.log(2, "My first plugin has successfully activated!");
    return 0;
}

} // extern "C"
```

## Step 2: The CMake Definition

You must compile this as a dynamic library that exports the C-ABI symbols.

```cmake
cmake_minimum_required(VERSION 3.20)
project(my_plugin_project CXX)

add_library(my_plugin SHARED my_plugin.cpp)
set_target_properties(my_plugin PROPERTIES CXX_STANDARD 23)

# Link against the engine's public headers
target_include_directories(my_plugin PRIVATE ${SANDBOX_API_INCLUDE_DIR})
```

## Step 3: Write the Manifest

The engine relies on a `manifest.json` to topologically sort and load your plugin. Create a `manifest.json` in the same directory:

```json
{
    "modules": [
        {
            "id": "my_plugin",
            "version": "1.0.0",
            "entry_point": "my_plugin",
            "requirements": [
                { "id": "core_logger", "kind": "import", "strictness": "required" }
            ]
        }
    ]
}
```

## Step 4: Run the Engine

Package your `.so` (or `.dll`) and the `manifest.json` into a folder named `app`. Run the engine:

```bash
./sandbox --mount ./app --run
```

You should see the output: `[info] My first plugin has successfully activated!`
""",

    "docs/tutorials/02_ecs_systems.md": """# Tutorial: ECS Systems & Components

The Sandbox Engine uses [Flecs](https://github.com/SanderMertens/flecs), an extremely fast Entity Component System (ECS).

## Step 1: Defining a Component

Components are standard C++ structs.

```cpp
struct Transform {
    float x;
    float y;
};
```

## Step 2: Registering in a Plugin

In your `activate` function, register the component and define a system that operates on it.

```cpp
#include <sandbox/core/plugin.h>

struct Transform { float x, y; };
struct Velocity { float dx, dy; };

extern "C" {

SANDBOX_DECLARE_MODULE(movement_system)

int32_t movement_system_activate(flecs::world& ecs, const module_info* info) {
    
    ecs.component<Transform>();
    ecs.component<Velocity>();

    // Define a system that updates Transforms based on Velocities
    ecs.system<Transform, const Velocity>("MovementSystem")
        .each([](flecs::entity e, Transform& t, const Velocity& v) {
            t.x += v.dx;
            t.y += v.dy;
        });

    return 0;
}

}
```

You can now spawn entities in this plugin or any other plugin that requires `movement_system`!
""",

    "docs/how-to/01_use_event_bus.md": """# How-To: Use the Event Bus

The Event Bus allows cross-plugin communication without direct C++ linkage. Payloads are defined using FlatBuffers.

## 1. Subscribing to an Event

Use `sandbox::subscribe`.

```cpp
#include <sandbox/utilities/events.h>
#include "my_event_generated.h"

int32_t my_plugin_activate(flecs::world& ecs, const module_info* info) {
    sandbox::subscribe(ecs, "player_jump_event", [](const uint8_t* payload_data, size_t size) {
        auto fb = flatbuffers::GetRoot<MyGame::PlayerJump>(payload_data);
        float force = fb->force();
        // Handle jump logic...
    });
    return 0;
}
```

## 2. Publishing an Event

Use `sandbox::publish`.

```cpp
#include <sandbox/utilities/events.h>
#include "my_event_generated.h"

void trigger_jump(flecs::world& ecs) {
    flatbuffers::FlatBufferBuilder builder;
    auto jump_event = MyGame::CreatePlayerJump(builder, 15.0f);
    builder.Finish(jump_event);

    sandbox::publish(ecs, "player_jump_event", builder.GetBufferPointer(), builder.GetSize());
}
```
""",

    "docs/how-to/02_read_write_vfs.md": """# How-To: Read and Write to the Virtual File System

PhysicsFS provides a jailed Virtual File System (VFS). All file access must occur through `mount://` URIs.

## Reading a File

Use the SDK wrapper to ensure ABI safety.

```cpp
#include <sandbox/modules/filesystem/filesystem_api.h>
#include <iostream>

void read_config(flecs::world& ecs) {
    sandbox::sdk::filesystem fs(ecs);
    
    auto res = fs.read_text("mount://app/config.json");
    if (res.has_value()) {
        std::cout << "Config: " << res.value() << std::endl;
    } else {
        std::cerr << "Error: " << res.error() << std::endl;
    }
}
```

## Writing a File

```cpp
#include <sandbox/modules/filesystem/filesystem_api.h>

void write_save(flecs::world& ecs, const std::string& data) {
    sandbox::sdk::filesystem fs(ecs);
    
    auto res = fs.write("mount://cache/save1.dat", data);
    if (!res.has_value()) {
        std::cerr << "Failed to save: " << res.error() << std::endl;
    }
}
```
""",

    "docs/how-to/03_define_flatbuffer_schemas.md": """# How-To: Define FlatBuffer Schemas

To pass complex data across the ABI boundary, you must define a `.fbs` schema.

## 1. Create the Schema

Create `player_events.fbs`:

```fbs
namespace MyGame.Events;

table PlayerJump {
  force: float;
  is_double_jump: bool;
}

root_type PlayerJump;
```

## 2. Compile the Schema

Use `flatc` to generate the C++ headers.

```bash
flatc --cpp player_events.fbs
```

This generates `player_events_generated.h` which you can include in your plugin to serialize and deserialize the payload data.
""",

    "docs/reference/01_manifest_json.md": """# Reference: Manifest.json

The `manifest.json` file is required for the bootstrapper to locate and load modules.

## Structure

```json
{
  "modules": [
    {
      "id": "my_module",
      "version": "1.0.0",
      "entry_point": "my_module_shared_lib",
      "requirements": [
        {
          "id": "core_logger",
          "kind": "import",
          "strictness": "required"
        }
      ]
    }
  ]
}
```

## Fields

- `id`: The unique identifier for the module.
- `version`: Semantic version string.
- `entry_point`: The name of the shared library file (without the `.dll` or `.so` extension). The engine will automatically append the correct OS-specific extension.
- `requirements`: An array of dependencies.
  - `id`: The target module ID.
  - `kind`: `import` (must run before this module) or `export` (runs after this module).
  - `strictness`: `required` (fail if missing) or `optional` (continue if missing).
""",

    "docs/reference/02_launcher_args.md": """# Reference: Launcher CLI Arguments

The Sandbox Engine launcher accepts several command-line arguments.

```text
Usage: ./bin/sandbox [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
  -m,--mount TEXT:PATH        (REQUIRED) Path to the application archive or directory.
                              Can be a folder or an archive (like .zip).
  -d,--dev                    Enable developer mode (verbose logging and tracing).
  -r,--run                    Run the engine main loop immediately after boot.
  -p,--prop [TEXT,TEXT] ...   Custom module properties in Key=Value format.
```

## Examples

Run a zipped game archive:
```bash
./sandbox --mount my_game.zip --run
```

Run in developer mode with custom properties:
```bash
./sandbox --mount ./game_data/ --dev -p fps_limit=60 -p logger_level=1
```
""",

    "docs/reference/03_core_services_api.md": """# Reference: Core Services API

The engine provides core C-ABI services via the `execute_command` pattern. You interact with them via SDK wrappers.

## Logger (`sandbox::sdk::logger`)
- `log(int level, const std::string& msg)`
- `set_property(const std::string& key, const T& value)`

## Filesystem (`sandbox::sdk::filesystem`)
- `read_text(const std::string& path)` -> `std::expected<std::string>`
- `read_binary(const std::string& path)` -> `std::expected<std::vector<std::byte>>`
- `write(const std::string& path, const std::string& data, bool append)`
- `list(const std::string& path)` -> `std::expected<std::vector<std::string>>`
- `state(const std::string& path)` -> `file_state { size, is_directory, modified_time }`
- `mount(const std::string& physical, const std::string& virtual_prefix)`

## Runner (`sandbox::sdk::runner`)
- `set_property("fps_limit", float)`
""",

    "docs/reference/04_vfs_formats.md": """# Reference: Supported VFS Formats

The Sandbox Engine mounts applications using PhysicsFS. An application can be a raw folder or a compressed archive.

The `sandbox` launcher's `--mount` argument accepts the following formats:

1. **Directories**: Standard OS folders.
2. **ZIP** (`.zip`): Standard ZIP archives.
3. **7zip** (`.7z`): 7-Zip compression archives.
4. **GRP** (`.grp`): Build Engine archives (Duke Nukem 3D).
5. **WAD** (`.wad`): Doom engine archives.
6. **HOG** (`.hog`): Descent I/II archives.
7. **MVL** (`.mvl`): Descent II multiplayer archives.
8. **QPAK** (`.pak`): Quake I/II archives.
9. **SLB** (`.slb`): I-War / Independence War archives.
10. **VDF** (`.vdf`): Gothic I/II archives.
11. **ISO9660** (`.iso`): CD-ROM images.

*Note: For production releases, a `.zip` file is the highly recommended standard.*
""",

    "docs/explanation/01_meta_engine_paradigm.md": """# Explanation: The Meta-Engine Paradigm

The Sandbox Engine is fundamentally a **Meta-Engine**. It is an engine designed to build other engines.

## The Problem
Traditional game engines are monolithic. If you want to replace the renderer, the physics system, or the scripting language, you must fork the engine and recompile it. This leads to rigid architectures.

## The Solution
The Sandbox Engine acts purely as a `Bootstrapper` and a `Registry`. It provides zero gameplay logic, zero rendering, and zero physics. Instead, it provides a strictly ordered DAG (Directed Acyclic Graph) of dynamic libraries (`.dll` / `.so`).

Every subsystem—even the core logger and virtual filesystem—is a plugin. This allows developers to hot-swap out the core systems by simply modifying the `manifest.json`.

If an application needs an Unreal-style architecture, it loads modules that establish that architecture. If it needs a minimalist 2D framework, it loads entirely different modules. The engine is a blank slate dictated entirely by the loaded plugins.
""",

    "docs/explanation/02_c_abi_wall.md": """# Explanation: The C-ABI Firewall

In C++, passing objects across DLL boundaries is catastrophically dangerous.

## The DLL Boundary Problem
If `Plugin A` allocates a `std::vector` and passes it to `Plugin B`, and `Plugin B` modifies or frees it, the program will likely crash. This is because:
1. `Plugin A` and `Plugin B` might be compiled with different heap allocators.
2. They might use different standard libraries (e.g., MSVC vs. libstdc++).
3. They might be compiled with different C++ versions (C++20 vs C++23).
4. Exceptions thrown in `Plugin A` cannot be safely caught in `Plugin B`.

## The Sandbox Solution: The C-ABI Wall
The Sandbox Engine forces all communication between plugins to occur over a strict **C-ABI Firewall**.

All services must be defined as C-structs containing a single function pointer: `execute_command`.

```cpp
struct my_service {
    void* instance;
    void (*execute_command)(void* instance, uint32_t command, const uint8_t* payload, size_t size);
};
```

This enforces that only primitive integers, raw memory pointers, and flat byte arrays (FlatBuffers) can cross the boundary. 

To maintain modern C++ ergonomics, we use **SDK Wrappers** (`sandbox::sdk::*`) that internally serialize the arguments into FlatBuffers, invoke the C-ABI function, and deserialize the result into `std::expected`. This gives you the safety of C and the elegance of C++23.
""",

    "docs/explanation/03_kahn_sort_bootstrapper.md": """# Explanation: Kahn's Sort & The Bootstrapper

When the engine starts, it reads the `manifest.json` to discover all required plugins. However, plugins have dependencies. `Renderer` might depend on `Window`, which depends on `Logger`.

## Topological Sorting
The Bootstrapper uses **Kahn's Algorithm** to perform a topological sort on the dependency graph.

1. It calculates the "in-degree" (number of incoming dependencies) for every module.
2. It finds all modules with an in-degree of 0 (no dependencies) and pushes them to a queue.
3. It pops a module, activates it, and reduces the in-degree of all modules that depended on it.
4. If a module's in-degree hits 0, it is added to the queue.

This guarantees that a module is *never* loaded before its dependencies are fully initialized and registered in the ECS.

## Circular Dependencies
If Kahn's Algorithm completes but there are still modules left with an in-degree greater than 0, the Bootstrapper immediately halts and throws a `boot_error`. This prevents infinite loops caused by circular dependencies.
""",

    "docs/explanation/04_memory_ownership_flatbuffers.md": """# Explanation: Memory Ownership & FlatBuffers

Because of the C-ABI Firewall, memory ownership must be strictly managed when passing data.

## The Payload Struct
When a plugin requests data (e.g., reading a file), the engine returns a `sandbox_payload`.

```cpp
struct sandbox_payload {
    uint8_t* bytes;
    size_t size;
    void (*free_func)(void*);
};
```

**Rule 1: The Allocator Provides the Free Function**
Whichever DLL allocates the `bytes` must provide the `free_func` pointer. When the receiving DLL is finished with the data, it invokes `free_func(bytes)`. This ensures that memory is always freed by the allocator that created it, avoiding cross-heap corruption.

## FlatBuffers for IPC
To pass structured data, the engine uses **FlatBuffers**. 

FlatBuffers are zero-copy memory-mapped data structures. When an event is published via the Event Bus, the `FlatBufferBuilder` constructs a contiguous byte array. This raw array is passed across the C-ABI wall. The receiving plugin casts the raw byte pointer using `flatbuffers::GetRoot<T>()` and instantly reads the structured data without any deserialization overhead.
"""
}

for filepath, content in files.items():
    os.makedirs(os.path.dirname(filepath), exist_ok=True)
    with open(filepath, 'w') as f:
        f.write(content)

print("Documentation generated successfully.")
