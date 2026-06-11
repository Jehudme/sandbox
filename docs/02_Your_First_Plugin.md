# 2. Your First Plugin ("Hello World")

*This visceral, step-by-step tutorial walks you through creating a basic `.dll` / `.so` plugin from scratch.*

**Conclusion first:** To build a plugin, you must define a C++ struct that accepts a `flecs::world&`, log a message using `SANDBOX_INFO`, declare the module dependencies with `SANDBOX_DECLARE_MODULE`, and finally export the ABI-safe entry point using `SANDBOX_DECLARE_LIBRARY()`.

## Step 1: The Basic C++ Struct
A Sandbox plugin is simply a C++ struct. The engine will instantiate this struct during the Bootstrapper's execution phase, passing it a reference to the global ECS world.

Create a file named `hello_world.cpp`:

```cpp
#include <sandbox/core/plugin.h>
#include <sandbox/api/logger_api.h>

struct hello_world_module {
    // The constructor is your plugin's "Main" function.
    hello_world_module(flecs::world& ecs) {
        
        // Use the global macro to send an ABI-safe FlatBuffer log message
        SANDBOX_INFO(ecs, "Hello World from my first plugin!");
        
    }
};
```

## Step 2: Declare Dependencies
The bootstrapper needs to know about your module *before* it initializes it. You use the `SANDBOX_DECLARE_MODULE` macro at the bottom of your file. 

We will name the plugin `hello_world`, version it `1.0.0`, and declare that it absolutely *requires* the `logger_service` to exist first.

```cpp
// 1. Struct Name: hello_world_module
// 2. Export Name: hello_world
// 3. Version: 1.0.0
// 4. Provided Service: "" (None)
// 5. Dependencies...
SANDBOX_DECLARE_MODULE(hello_world_module, hello_world, 1, 0, 0, "",
    {sandbox::requirement::kind::service, sandbox::requirement::strictness::require, "logger_service", 1, 0}
);
```
*Note: This macro implicitly generates the `get_sandbox_module_info` C-export required by the Bootstrapper.*

## Step 3: The DLL Entry Point
Finally, Flecs and the OS loader need a specific function symbol to execute when `dlopen` (or `LoadLibrary`) is called. Add the `SANDBOX_DECLARE_LIBRARY()` macro to the very end of `hello_world.cpp`.

```cpp
SANDBOX_DECLARE_LIBRARY()
```
*Note: This macro implicitly defines the `SandboxLibraryMain` (or `load_sandbox_plugin` depending on internal macros) that the engine executes.*

## Step 4: The Manifest
For the engine to load your compiled `.so`, you must create a `manifest.json` file in the root of your application folder.

```json
{
  "modules": {
    "hello_world": "1.0"
  }
}
```

## Step 5: Run It
Assuming you compiled `hello_world.cpp` into `hello_world.so` and placed it in your app's `modules/` folder:

```bash
./sandbox --mount ./my_app_folder --run
```
You should see: `[info] Hello World from my first plugin!`
