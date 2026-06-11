# 1.  The Core Philosophy & Architectural Rules

## The Meta-Engine Paradigm
The Sandbox Engine does not exist as a traditional monolithic application. Instead, it operates under the **Meta-Engine Paradigm**: the vision of an engine as a mathematical graph of isolated dynamic libraries (`.so` or `.dll`).

In this paradigm, the engine's executable (`sandbox`) is incredibly lightweight. It acts exclusively as a topological loader and an Entity Component System (ECS) host. Every major piece of logic—rendering, physics, scripting, and even the core VFS and Logger—exists as independent plugins that are resolved, verified, and chained together dynamically at runtime.

## The "Public vs. Private" Firewall
To maintain stability, the engine code is strictly divided:
*   **`sandbox/src/` (Private):** Contains the core implementation (`engine.cpp`, `bootstrapper.cpp`, `logger.cpp`). Plugins **never** include headers from this folder. Attempting to do so breaks the firewall and destroys ABI compatibility.
*   **`sandbox/include/sandbox/api/` (Public):** The sanitized surface area exposed to plugins. It contains flat C-structures (`abi_types.h`) and standard C++23 wrappers (`sdk_wrappers`) designed to be safely compiled by the plugin's toolchain.

You should **never** expose internal C++ interfaces (like `ilogger.h` or `ifilesystem.h`) to plugins. Doing so forces the plugin to rely on the exact vtable layout and compiler ABI of the core engine, which is a recipe for disaster.

## The C-ABI Boundary (The Wall)
Modern C++ is notorious for its lack of a stable Application Binary Interface (ABI).
*   If the engine is compiled with GCC 11 and a plugin is compiled with Clang 14, passing a `std::string` or `std::vector` across the DLL boundary will almost certainly cause **catastrophic memory corruption** due to differing memory layouts (like Small String Optimization).
*   Similarly, throwing an exception (e.g., `throw std::runtime_error`) across a DLL boundary will unwind the stack into foreign memory space, causing an immediate `std::terminate` or a segfault.

### How the Engine Enforces the Wall
The Sandbox Engine solves this by enforcing a strictly raw C-pointer boundary at the edges:
1.  **Pure C Structs:** Services (`logger_service`, `filesystem_service`) are defined purely as function pointers in `abi_types.h`.
2.  **Opaque Instances:** Context is passed via an opaque `void* instance` pointer.
3.  **No Exceptions:** Functions return integer error codes (`0` for success, non-zero for failure).
4.  **FlatBuffers for Data:** When complex data must cross the boundary, it is serialized into a flat byte array (`sandbox_payload`), passed as a raw pointer/size pair, and safely deserialized on the other side.

The provided `sandbox::sdk` wrappers sit purely on the plugin's side of the wall. They wrap the raw C function pointers and elevate the raw integer error codes into modern C++ `std::expected` types, giving you safety without sacrificing the ABI wall.

## Memory Ownership Strategy
When building isolated plugins, defining *who owns what* is crucial to avoiding double-free corruption:

1.  **The Plugin owns its Heap:** Any memory allocated by the plugin (`new`, `malloc`, or internal `std::vector` capacity) must be freed by the plugin. The engine will never attempt to `delete` a pointer handed to it from a plugin.
2.  **The Engine owns the ECS:** The Flecs world memory is owned entirely by the core engine. Plugins receive a reference (`flecs::world&`) and are allowed to mutate it, register components, and spawn entities. However, the memory is managed centrally. When the engine shuts down, the ECS is cleanly wiped from the engine side.
3.  **Payload Callbacks:** When a `sandbox_payload` crosses the boundary, it contains a `void (*free_func)(void*)` pointer. If the engine allocates a FlatBuffer byte array, it provides the exact function the plugin must call to free it, ensuring memory is always deallocated by the same heap manager that allocated it.
