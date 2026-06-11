# 2.  The Module Graph & Bootstrapper

The `bootstrapper` subsystem (`sandbox/src/core/bootstrapper.cpp`) is the architectural heart of the engine's plugin resolution. It guarantees that modules boot in a safe, mathematically predictable order.

## Kahn’s Topological Sort
Plugins declare what they *require* via the `SANDBOX_DECLARE_MODULE` macro. For example, the `audio` plugin might require the `filesystem_service` and `logger_service`.

If `audio` initializes before `logger`, it will crash trying to log its startup sequence.
To solve this, the bootstrapper builds a Directed Acyclic Graph (DAG) of dependencies and executes **Kahn's Algorithm** for topological sorting.

1.  **In-Degree Calculation:** The bootstrapper maps every activated plugin as a node. It assigns an "in-degree" (the number of dependencies it waits on) to each node.
2.  **Queue Processing:** Modules with an in-degree of `0` (no dependencies, like the core `logger`) are booted first.
3.  **Graph Resolution:** Once a module boots successfully, the bootstrapper decrements the in-degree of any modules waiting on it. 
4.  **Cyclic Graph Detection:** If Kahn's algorithm finishes but un-booted modules remain in the queue, a **circular dependency** has been detected (e.g., A requires B, and B requires A). The bootstrapper immediately halts the engine with a fatal exception to prevent infinite loops.

## The Module ABI Contract
For the bootstrapper to inspect a `.so` or `.dll`, it must understand its metadata *without* executing its internal logic.
This is achieved via the mathematical requirement of the `get_sandbox_module_info` and `get_sandbox_service_info` local registries.

When you use `SANDBOX_DECLARE_MODULE`, you are injecting a static initializer that pushes a `module_info` struct into a global C++ vector *inside the plugin's local memory space*. 

When the bootstrapper invokes the DLL entry point (`SandboxLibraryMain`), the plugin invokes `sandbox::detail::stage_library()`. This hands the bootstrapper a copy of the plugin's isolated local registry containing its name, version, and dependency list.

## The Injection Lifecycle
The exact sequence of loading is strictly ordered to prevent undefined behavior:

1.  **VFS Boot:** The engine mounts the `mount://app`, `mount://bin`, and `mount://cache` filesystems. It must do this *before* it can locate any `.so` plugins.
2.  **Manifest Parsing:** The engine reads `manifest.json` from the app mount, determining which modules are specifically requested by the developer.
3.  **Staging (`boot.stage`):** The engine searches the VFS for matching `.so` / `.dll` files. It uses `dlopen` (or `LoadLibrary`) to load them into memory and calls their `SandboxLibraryMain` entry point. The plugins push their metadata into the bootstrapper, but *do not execute game logic yet*.
4.  **Resolution (`boot.resolve_activations`):** The bootstrapper compares the requested modules in `manifest.json` against the staged metadata. It drops incompatible versions and audits service collisions (e.g., two plugins providing the `physics_service` will result in one being deterministically prioritized and a warning issued).
5.  **Execution (`boot.execute`):** Kahn's algorithm runs. The engine passes the `flecs::world&` context to each module's `import_fn` (their struct constructor) sequentially.
6.  **Unloading:** At engine shutdown, the OS unloads the dynamic libraries. The engine destroys the ECS, naturally wiping all components registered by the plugins.
