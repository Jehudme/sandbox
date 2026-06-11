# 4. 📖 THE ENGINE MANIFEST (Comprehensive Feature & File Reference)

This is the exhaustive dictionary of the Sandbox Meta-Engine codebase. It maps every macro, API wrapper, subsystem, and core component on a strict file-by-file basis.

## A. Global Macros & Preprocessor Directives

### `SANDBOX_EXPORT` / `SANDBOX_API`
*   **Location:** Found scattered in headers that define the C-ABI or library entry points.
*   **Purpose:** Ensures the compiler generates cross-platform dynamic linking symbols (`__declspec(dllexport)` on Windows, `__attribute__((visibility("default")))` on GCC/Clang). This is critical for enforcing the C-ABI boundary and ensuring OS loaders (like `dlopen`) can find the `SandboxLibraryMain` function.

### `INTERNAL_SANDBOX_LOG_PUBLISH`
*   **Location:** `logger_api.h`
*   **Details:** The hidden master macro that executes a FlatBuffer serialization of log messages. It builds a `sandbox::schemas::logger::LogMessage` payload, attaches the `__FILE__` and `__LINE__` metadata, and dispatches it over the ABI to the `logger_service`.
*   **WARNING:** This macro deliberately ignores standard error returns from the C-ABI because a logging failure usually means catastrophic memory state. It will throw a `sandbox::boot_error` natively on the host side if it fails.

### Standard Logging Macros
*   **Location:** `logger_api.h`
*   **Features:** `SANDBOX_INFO`, `SANDBOX_WARN`, `SANDBOX_ERROR`, `SANDBOX_FATAL`, `SANDBOX_TRACE`, `SANDBOX_DEBUG`.
*   **Throw Overrides:** Variants like `SANDBOX_FATAL_THROW` explicitly request the receiving logger service to crash the engine safely via an ABI request.

## B. API Wrappers (`include/sandbox/api/`)

These wrappers take the C structs from `abi_types.h` and provide C++23 `std::expected` safety, guarding against exceptions and raw pointer misuse.

### `logger_api.h`
*   **Features:** `log(level, string)`, `set_property()`, `get_property()`.
*   **Internal Detail:** Converts `std::string` messages to FlatBuffers and calls `m_api->log()`. The wrapper dynamically fetches the `logger_service` from the ECS.

### `filesystem_api.h`
*   **Features:** `read_text()`, `read_binary()`, `write()`, `mount()`, `unmount()`, `list()`, `mkdir()`, `remove()`, `rename()`, `copy()`, `move()`, `absolute()`, `state()`.
*   **Warnings:** Emphasizes that this API accepts **jailed paths** only (e.g., `mount://cache/save.dat`). Attempting to pass physical OS paths (e.g., `/etc/passwd` or `C:/Windows`) will be rejected by the VFS layer.

### `runner_api.h`
*   **Features:** `start_async()`, `run_sync()`, `pause()`, `resume()`, `quit()`.
*   **Usage:** Used to control the main thread lifecycle. `run_sync()` blocks and ticks the engine; `quit()` signals the internal loop to terminate.

### `payload.h` (`sandbox::sdk::payload`)
*   **Features:** A RAII wrapper for `sandbox_payload` (`abi_types.h`). It automatically invokes `m_payload.free_func(m_payload.bytes)` on destruction, preventing memory leaks when byte arrays cross the DLL wall.

## C. Core Engine Implementation (`src/core/`)

These files compile exclusively into the main `sandbox` executable and should never be exposed to plugins.

### `engine.cpp` & `engine.h`
*   **Features:** The Master PIMPL implementation. Holds the master `flecs::world`. Executes `import_core_infrastructure()` which mounts the engine's internal native plugins.
*   **Warnings:** Explicitly uses raw pointers (`impl* m_impl`) managed manually in the destructor. **Do not convert to `std::unique_ptr`**. Because plugins can hold references or trigger shutdowns from foreign threads/heaps, utilizing standard smart pointers across the PIMPL boundary has historically caused cross-DLL heap deletion crashes.

### `bootstrapper.cpp`
*   **Features:** Houses the Kahn's Algorithm logic, stage(), activate(), and execute() functions. Emits warning logs for module version collisions.
*   **Error Handling:** Throws native `sandbox::boot_error` exceptions, which are strictly caught by the launcher, terminating execution before standard loops begin.

### `plugin.h` & `plugin.cpp` (Internal Utility)
*   **Features:** The wrapper class that holds the OS handle to the loaded `.dll` / `.so`. 

### `properties.cpp`
*   **Features:** The JSON configuration tree parser, leveraging `glaze`. Evaluates `--prop` CLI arguments.
*   **Warnings:** Includes strict guard checks in `move()` that prevent cyclic nesting crashes (e.g., trying to move a node into its own child node).

## D. Subsystems & Interfaces (`src/subsystems/`)

The actual implementations behind the C-ABI service structures.

### Logger (`logger.cpp` / `ilogger.h`)
*   **Features:** Sinks data physically to the console via `spdlog`. Receives the `sandbox_payload` byte array, deserializes the FlatBuffer back into `std::string`, and logs it.
*   **Warnings:** The `ilogger` interface is completely private. Do not invoke virtual functions from `ilogger.h` directly in game code.

### Filesystem (`filesystem.cpp` / `ifilesystem.h`)
*   **Features:** Wraps PhysicsFS (PhysFS). Translates `mount://` aliases to physical host paths securely.
*   **Danger Zone (Path Traversal):** The `resolve_physical_write_path` securely sanitizes inputs. It intercepts and audits input using string iteration and strict parsing to prevent directory climbing attacks (`../../`).

### Runner (`runner.cpp` / `irunner.h`)
*   **Features:** The master tick loop. Calls `ecs.progress()`, calculates Delta Time, and throttles framerates based on `fps_limit` property.
*   **Danger Zone (Naked Threads):** Provides the top-level `try/catch` firewall enclosing the execution loop. This ensures that if a rogue C++ plugin violates the C-ABI rules and throws an exception during `ecs.progress()`, it is caught and gracefully logged rather than triggering a hard `std::terminate`.

## E. Utilities & Event Bus (`src/utilities/` & `src/event_bus/`)

### `event_bus.inl`
*   **Features:** Provides `publish_raw` and `subscribe_raw` using Flecs observers and `ChannelTag` entities to isolate event streams.
*   **Warnings:** Plugins publishing simultaneously on the same channel asynchronously require caution. `publish_raw_async` uses `world.defer()` to push execution to the safe ECS sync point.

### `loader.cpp` / `loader.h`
*   **Features:** Cross-platform OS abstraction for `LoadLibrary` (Windows) and `dlopen` (Linux/macOS).

### `config_helper.h`
*   **Features:** Convenience templates utilizing Glaze for fast, zero-allocation serialization between C++ structures and JSON files in the VFS.

## F. Data Schemas (`src/schemas/`)

### `.fbs` Files (FlatBuffers)
*   **Details:** Located in the generator directory. Files like `logger.fbs` and `filesystem.fbs` dictate the exact binary layout of the payloads crossing the ABI wall. Changes here must be regenerated with `flatc` before compiling.
