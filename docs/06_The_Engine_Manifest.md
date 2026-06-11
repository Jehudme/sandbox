# 6. The Engine Manifest (File Dictionary)

*This is an exhaustive dictionary of the Sandbox Meta-Engine codebase. It maps every macro, API wrapper, and core component.*

## A. Global Macros & Preprocessor Directives

*   `SANDBOX_API` **(abi_types.h):** Toggles `__declspec(dllexport)` vs `dllimport` based on build configurations to enforce the C-ABI boundary on Windows and Linux visibility attributes.
*   `INTERNAL_SANDBOX_LOG_PUBLISH` **(logger_api.h):** The FlatBuffer dispatch macro. It builds a `LogMessage` payload and fires it. **WARNING:** Drops the C-ABI integer return value, requiring native host-side exceptions (`sandbox::boot_error`) if the dispatch completely fails.
*   `SANDBOX_FATAL_THROW`: Executes a log message marked as Fatal and triggers a safe engine shutdown request.
*   `SANDBOX_DECLARE_MODULE`: Injects the `get_sandbox_module_info` C-export metadata requirement.
*   `SANDBOX_DECLARE_LIBRARY()`: Injects the `SandboxLibraryMain` Flecs entry point to invoke the module registration sequence.

## B. API Wrappers (`include/sandbox/api/`)

*   **`logger_api.h`:** Contains `sandbox::sdk::logger`. Features `log()`, `set_property()`. Packs strings into FlatBuffers and forwards to the ECS C-function pointer.
*   **`filesystem_api.h`:** Contains `sandbox::sdk::filesystem`. Features `read_text()`, `read_binary()`, `write()`, `mount()`. **Warning:** API only accepts jailed virtual paths (e.g., `mount://`).
*   **`runner_api.h`:** Contains `sandbox::sdk::runner`. Features `start_async()`, `run_sync()`, `pause()`, `quit()`.
*   **`payload.h`:** RAII memory management wrapper for `sandbox_payload` ensuring C-ABI flatbuffer arrays are automatically freed across boundaries using the stored `free_func` pointer.

## C. Core Engine Implementation (`src/core/`)

*   **`engine.cpp` & `engine.h`:** The Master PIMPL implementation. **Warning:** Developers are strictly warned NOT to convert the raw `impl* m_impl` pointer to a `std::unique_ptr` to avoid cross-DLL heap deletion crashes during destruction on shutdown.
*   **`bootstrapper.cpp`:** Executes Directory Scanning, LoadLibrary/dlopen, and Kahn's Algorithm for Topological Sorting of plugins.
*   **`properties.cpp`:** JSON configuration tree parser using Glaze. Features `get()`, `set()`, `move()`. Contains structural guards to prevent cyclic nesting crashes (e.g. moving a node into its own child).

## D. Subsystems & Interfaces (`src/subsystems/`)

*   **Logger (`logger.cpp` / `ilogger.h`):** Translates the FlatBuffer bytes back into string representations and formats output to the physical console via `spdlog`.
*   **Filesystem (`filesystem.cpp` / `ifilesystem.h`):** Powered by PhysicsFS. Translates `mount://` aliases to physical paths. Contains `resolve_physical_write_path`, enforcing strict iterator-based directory traversal checks (`../../`).
*   **Runner (`runner.cpp` / `irunner.h`):** The master tick loop. Contains a top-level `try/catch` firewall to intercept rogue C++ exceptions escaping poorly-written plugins before they trigger an uncontrolled `std::terminate`.

## E. Utilities & Event Bus (`src/utilities/` & `src/event_bus/`)

*   **`event_bus.inl`:** Template-based subscription routing wrapped around Flecs `observer()` logic.
*   **`loader.cpp` / `loader.h`:** Cross-platform dynamic library OS abstraction (`LoadLibrary`/`dlopen`).
*   **`config_helper.h`:** Glaze serialization wrapper macros.
