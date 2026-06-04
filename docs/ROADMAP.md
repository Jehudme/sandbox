🗺️ Sandbox Meta-Engine: v0.1.0 Architecture Roadmap

This document outlines the architectural roadmap to bring the Sandbox engine to a stable v0.1.0 MVP. The tasks are strictly ordered to minimize refactoring friction: Structural changes first, followed by stability fixes, paradigm shifts, advanced features, and finally quality assurance.
📂 Phase 1: Structural Refactoring & Naming Conventions

Goal: Reorganize the codebase into a predictable, professional SDK layout before adding new features. Move away from a "scripting" feel to a mature C++ library structure.

    [ ] 1.1. Subsystem Isolation & C++20 Disambiguation

        The Problem: The directory name modules/ is heavily overloaded in modern C++ and conflicts mentally with C++20 modules. Furthermore, implementations are clumped together, and exceptions/macros are isolated in disconnected folders.

        The Solution: 1. Rename source/modules/ to source/subsystems/.
        2. Create dedicated sub-directories for each major system (e.g., source/subsystems/logger/, source/subsystems/plugins/).
        3. Co-locate all related files. Move logger.h, logger.cpp, and related macros into the logger/ folder.
        4. Dissolve the include/sandbox/exceptions/ and include/sandbox/macros/ folders. Public exception types and macros should live at the bottom of the public component headers they relate to (e.g., vfs_exceptions goes into the public filesystem header).

    [ ] 1.2. Public SDK Event Bus Consolidation

        The Problem: The public-facing event headers lack clear naming conventions and expose messy internal template implementations to the end-user.

        The Solution: 1. Rename include/sandbox/events/ to include/sandbox/event_bus/.
        2. Suffix all event payload files with _events.h (e.g., vfs.h becomes filesystem_events.h).
        3. Move .inl template implementation files (like logger.inl and events.inl) into a nested detail/ folder to hide implementation details from the public SDK surface.

    [ ] 1.3. Filesystem Subsystem Renaming

        The Problem: "VFS" (Virtual File System) is an implementation detail (PhysFS). The engine and its users interact with it simply as the engine's "Filesystem".

        The Solution: Globally rename the vfs subsystem, macros (SANDBOX_VFS_*), and namespaces to filesystem for clarity and consistency with the C++ standard library.

🩹 Phase 2: Immediate Stability & Bug Fixes

Goal: Fast, high-impact fixes that eliminate immediate technical debt, fatal runtime collisions, and silent failures.
    [ ] 2.1. Multi-Instance Engine Support (The Spdlog Collision)

        The Problem: In logger.cpp, the logger is instantiated using a hardcoded string: std::make_shared<spdlog::logger>("sandbox_core", ...) and subsequently registered globally via spdlog::register_logger(). If an application attempts to spin up two sandbox::engine instances, the second instance will throw a fatal exception because "sandbox_core" already exists in the global registry. Furthermore, calling spdlog::drop("sandbox_core") in one engine's destructor will destroy the other engine's logger.

        The Solution: Stop using a hardcoded string and the global registry. Generate a unique name per engine instance (e.g., "sandbox_core_" + uuid), OR do not call spdlog::register_logger() at all. Just hold the std::shared_ptr<spdlog::logger> locally inside the engine instance and use it directly.

    [ ] 2.2. Unified Configuration Pipeline

        The Problem: CLI arguments and internal engine properties (from JSON) are handled via separate pipelines, making it difficult for plugins to access a single source of truth for configuration.

        The Solution: Parse all CLI arguments and inject them into a unified sandbox::properties object first. Pass this single JSON-backed property tree into engine::initialize(), acting as the master configuration state.

    [ ] 2.3. Runner Exception Safety Net

        The Problem: The runner module executes the main tick loop without an overarching safety net. Any unhandled exception thrown by a plugin will bubble up and violently crash the host OS process without flushing logs.

        The Solution: Wrap the runner's internal internal_tick_loop in a top-level try-catch block. Catch const std::exception&, flush the engine logger with a [FATAL] trace, and trigger a graceful engine shutdown sequence.

    [ ] 2.4. Fundamental Utilities Expansion

        The Problem: The engine lacks the high-performance core utilities necessary for game logic.

        The Solution: Implement a <random> generation wrapper using a fast algorithm (e.g., PCG or Xoshiro256) and create basic mathematical primitives (Vectors, Matrices) tailored for the ECS.

⚙️ Phase 3: Core C++23 Paradigm Shifts

Goal: Modernize the C++ architecture to eliminate threading bottlenecks, unnecessary heap allocations, and control-flow disruptions.

    [ ] 3.1. Eradicate "Events as RPC" (Remote Procedure Calls)

        The Problem: The engine abuses the Event Bus to request data. Currently, reading a file involves firing a read_request event containing a mutable std::function callback, which the caller must evaluate. This is thread-unsafe, slow, and convolutes control flow.

        The Solution: Build a dedicated Service Locator or RPC Subsystem for blocking data requests. If a system needs to read a file, it should invoke an API directly: auto data = engine.rpc().invoke("filesystem.read", path);. Reserve the Event Bus strictly for one-way, fire-and-forget state broadcasts (e.g., WindowResizedEvent, EntityDestroyedEvent).

    [ ] 3.2. Modern Error Handling (std::expected)

        The Problem: The engine relies on C++ exceptions for predictable, expected failures (e.g., a missing file, or a failed plugin load). Exceptions cause massive stack-unwinding overhead.

        The Solution: Migrate standard subsystem APIs to use C++23 std::expected<T, Error>. For example, filesystem::read should return std::expected<std::vector<std::byte>, fs_error_code>. This forces the caller to handle the failure at compile time. Reserve throw explicitly for unrecoverable logic errors or out-of-memory states.

    [ ] 3.3. Compile-Time String Interning (StringID)

        The Problem: Passing std::string around for property keys, event names, and ECS tags causes constant heap allocations and cache misses during comparison.

        The Solution: Implement a StringID class that utilizes a consteval or constexpr hashing algorithm (like FNV-1a). Convert string keys into uint32_t or uint64_t at compile time, drastically improving memory bandwidth and lookup speeds.

    [ ] 3.4. ABI Boundary Stabilization

        The Problem: Passing complex STL types (std::vector, std::string, std::function) across .dll or .so boundaries from the engine to a plugin can cause heap corruption if the engine and plugin are compiled with differing compiler flags or standard library versions.

        The Solution: Transition public plugin-facing APIs to use ABI-safe C-wrappers or raw pointer/size pairs (e.g., passing const char* and size_t instead of std::string).

🚀 Phase 4: Advanced Systems Implementation

Goal: Implement the heavy-hitting architecture required for a professional, highly scalable game engine.

    [ ] 4.1. Flecs-Powered Job System (Multithreading)

        The Problem: The engine currently runs on a single thread. Relying on external graphing libraries (like Taskflow) for ECS system execution adds unnecessary bloat.

        The Solution: Integrate Flecs' built-in Job System (ecs_set_threads). Organize systems into Flecs Phases (PreUpdate, OnUpdate, PostUpdate). Let the Flecs scheduler automatically multithread your systems based on their component read/write access dependencies.

    [ ] 4.2. Base Module Interface & Native Dependency Resolution

        The Problem: Modules lack a standardized interface, and resolving dependencies between them is manual and error-prone.

        The Solution: 1. Create a module_base abstract class with predictable lifecycle hooks (on_configure(), on_initialize(), on_shutdown()).
        2. Leverage Flecs' native module tracking (world.import<Module>()) to handle the dependency graph automatically, resolving initialization order dynamically.

    [ ] 4.3. Dynamic Manifest Reconfiguration (Hot-Reloading)

        The Problem: The engine reads manifest.json exactly once at boot. To tweak properties, developers must restart the entire application.

        The Solution: Subscribe the configuration subsystem to a FileModifiedEvent. When manifest.json changes on disk, hot-reload the properties tree and broadcast a ManifestReconfiguredEvent so all live subsystems can adjust their state immediately.

    [ ] 4.4. Logical File Security (Sandbox Jailing)

        The Problem: Allowing user-generated plugins unrestricted access to the OS filesystem is a massive security vulnerability (path traversal attacks).

        The Solution: Ensure every requested virtual path (mount://app/...) is pushed through std::filesystem::path::lexically_normal(). Verify that the resulting physical path strictly resides within the registered mount root before hitting the OS API. Explicitly reject any strings containing ../ that attempt to escape the designated boundary.

🧪 Phase 5: Quality Assurance & CI/CD

Goal: Ensure the engine remains mathematically and structurally sound as it scales across multiple operating systems.

    [ ] 5.1. Automated Testing Suite Integration

        The Problem: The meta-engine lacks a testing framework. Refactoring core subsystems currently carries a high risk of introducing silent regressions.

        The Solution: Integrate Catch2 or GoogleTest. Write comprehensive unit tests targeting:

            The JSON Properties parser.

            Filesystem logical jailing (actively test ../ attacks to ensure they are blocked).

            Event Bus and RPC routing.

    [ ] 5.2. Continuous Integration (CI) Pipeline

        The Problem: Relying on local desktop compilation hides platform-specific bugs (e.g., code that links successfully on Windows MSVC but fails on Linux GCC).

        The Solution: Create a .github/workflows/build.yml file. Configure GitHub Actions to automatically pull dependencies via your FetchContent setup and run your unit tests on Ubuntu, macOS, and Windows runners for every pushed commit or Pull Request.