# Professional Architectural & Technical Audit
**Target:** Sandbox Meta-Engine Core  
**Role:** Principal C++ Engine Architect  
**Date:** 2026-06-05

---

## 1. Executive Summary

**Production Readiness Score: 6.5 / 10**

The engine has undergone significant recent improvements (fixing dead code, RAII leaks, and namespace collisions), pulling it out of the "prototype" phase and into a structured architectural foundation. The use of Flecs for ECS, PhysFS for virtual file systems, and Spdlog for logging demonstrates a solid grasp of high-performance third-party integration. The topological module bootstrapper is a standout feature, correctly identifying and resolving dependency graphs.

However, the engine is not yet commercial-ready. Several deeply embedded anti-patterns remain—specifically regarding thread safety in the runner, hardcoded configuration values, ABI exposure across DLL boundaries, and inconsistent error propagation. The architecture is currently "brittle but functioning." Addressing the findings in this report will elevate the codebase to a true "Strong Guarantee" standard.

---

## 2. System-by-System Analysis

### [Runner Subsystem] `runner.cpp` / `runner.h`
*   **[CRITICAL] Thread-Safety and Exception Propagation:** The `internal_tick_loop` runs on a detached `std::thread` but contains no `try/catch` block. If `ecs.progress()` or any system within the ECS throws an unhandled exception, `std::terminate` will be invoked, crashing the application instantly without a crash dump or log flush.
*   **[WARNING] Hardcoded Framerate:** `ecs.set_target_fps(60);` is hardcoded. An engine should never dictate framerates internally; this must be driven by `sandbox::properties` or engine initialization arguments.
*   **[SUGGESTION] Condition Variable Predicate:** `pause()` works by changing `m_state` and relying on the `wait()` predicate `m_state == Running || m_state == Quitting` to block the thread. While functional, it is conceptually fragile. A dedicated `should_pause` flag or an explicit pause-yield loop is more idiomatic and easier to debug.

### [Core] `engine.h` / `engine.cpp`
*   **[WARNING] ABI Fragility:** `flecs::world ecs;` is exposed as a `public` member in a `SANDBOX_API` exported class. Any change to the `flecs` layout, or differing compiler flags between the engine and a plugin, will cause memory corruption. The ECS instance must be hidden behind a PIMPL (Pointer to Implementation) or an opaque accessor.
*   **[WARNING] Static Initialization Order Fiasco (SIOF) Risk:** The plugin OS API configuration `sandbox::configure_plugin_os_api()` is invoked manually in `main.cpp`. If a static module initializes before `main()` executes and attempts to interact with Flecs DLL loading, it will fail or crash.
*   **[SUGGESTION] Const Correctness:** `engine::arguments` members are mutable, but `initialize()` takes a `const arguments&`. This is correct, but the struct itself could enforce constness on its members if they are read-only post-construction.

### [Utilities] `properties.cpp` / `properties.h`
*   **[WARNING] Silent Failures in API:** `get_subtree()` and `list_keys()` return empty/default-constructed objects when a path is not found, rather than returning `std::unexpected`. This makes it impossible for the caller to distinguish between an empty JSON object and a completely invalid path.
*   **[WARNING] Self-Destructive Move:** `move(source, dest)` does not check if `dest` is a child of `source`. Moving a node into its own subtree causes cyclic or invalid JSON state (and likely a memory leak or crash in Glaze).
*   **[SUGGESTION] `std::string_view` Lifecycle:** In `load_from_bytes`, casting raw bytes to `std::string_view` assumes the Glaze parser will not read past the bounds. While Glaze is safe with bounded strings, it is standard practice to ensure JSON buffers are null-terminated or padded for SIMD parsing algorithms.

### [Plugin System] `plugin.h`
*   **[WARNING] Hacky Flecs Integration:** The macro `SANDBOX_DECLARE_LIBRARY` declares a dummy struct (`SandboxLibraryMain_Dummy`) purely to satisfy Flecs' strict import rules. This is an architectural smell. Flecs supports native C-API module registration which bypasses the need for dummy C++ types.
*   **[SUGGESTION] Macro Bloat:** The `SANDBOX_DECLARE_MODULE` macro defines a static lambda to register modules. It is clean, but macros hide debugging information. A template-based auto-registration system using CRTP (Curiously Recurring Template Pattern) would provide better compile-time type safety.

---

## 3. Anti-Pattern Report

### 1. The "Naked Thread" Pattern
Spawning `std::thread` without top-level exception handlers is a severe violation of production-grade C++. In a commercial engine, every thread must have a top-level `catch (...)` to safely initiate a shutdown sequence or generate a crash dump.

### 2. "Magic Numbers" in Core Logic
Hardcoding `60` FPS in the runner subsystem violates Separation of Concerns. The runner's job is to execute ticks; the application configuration's job is to define the rate.

### 3. ABI Boundary Exposure
Exporting a complex, third-party template class (`flecs::world`) across a DLL boundary (`SANDBOX_API engine`) guarantees ABI breakage. If the client app compiles with a different standard library implementation (e.g., libc++ vs libstdc++) or different optimization flags, `sizeof(flecs::world)` will mismatch, leading to silent memory overwrites.

---

## 4. Refactoring Roadmap

To reach a "Production-Ready" 9.0+ standard, prioritize the following roadmap:

1.  **Immediate (Stability):** Wrap the `internal_tick_loop` in a global exception handler. Flush the logger if an exception is caught.
2.  **Immediate (Architecture):** Encapsulate `flecs::world ecs;` inside `engine.cpp` using the PIMPL idiom. Provide a `get_world()` accessor returning a reference.
3.  **Short-term (Configuration):** Bind the Runner FPS to the `properties` subsystem. Read `TickRate` from `manifest.json`.
4.  **Short-term (API Health):** Update `properties::get_subtree()` to return `std::unexpected` on missing keys.
5.  **Long-term (Plugin Tech):** Remove the `SandboxLibraryMain_Dummy` hack in `plugin.h` and use `ecs_import_c`.

---

## 5. Actionable Feedback

### A. Exception Safety in Background Threads
**[BEFORE]** `runner.cpp`
```cpp
void runner::internal_tick_loop(world& ecs) {
    ecs.set_target_fps(60);
    while (true) {
        // ... wait logic ...
        if (!ecs.progress()) { quit(); }
    }
}
```

**[AFTER]** `runner.cpp`
```cpp
void runner::internal_tick_loop(world& ecs) noexcept {
    // Read from engine configuration
    ecs.set_target_fps(ecs.get<sandbox::engine_config>()->target_fps);
    
    try {
        while (true) {
            // ... wait logic ...
            if (!ecs.progress()) { quit(); }
        }
    } catch (const std::exception& e) {
        SANDBOX_FATAL(ecs, "[Runner] Unhandled exception in tick loop: {}", e.what());
        quit();
    } catch (...) {
        SANDBOX_FATAL(ecs, "[Runner] Unknown fatal exception in tick loop!");
        quit();
    }
}
```

### B. ABI-Safe Engine Declaration
**[BEFORE]** `engine.h`
```cpp
class SANDBOX_API engine {
public:
    flecs::world ecs; // <-- FATAL: ABI boundary violation
};
```

**[AFTER]** `engine.h`
```cpp
class SANDBOX_API engine {
public:
    flecs::world& get_ecs();
private:
    struct impl;
    std::unique_ptr<impl> m_impl; // PIMPL idiom hides third-party layouts
};
```

### C. Proper Error Propagation in Properties
**[BEFORE]** `properties.cpp`
```cpp
std::expected<properties, std::string> properties::get_subtree(const key_path& path) const {
    // ... loop ...
    } else return subtree_result; // Returns empty object silently
}
```

**[AFTER]** `properties.cpp`
```cpp
std::expected<properties, std::string> properties::get_subtree(const key_path& path) const {
    // ... loop ...
    } else return std::unexpected("Path not found: " + key);
}
```
