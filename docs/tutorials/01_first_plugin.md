# Tutorial: Your First Plugin

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
