# Sandbox Engine

A modern, highly modular, C-ABI safe dynamic meta-engine built in C++23.

## Overview
The Sandbox Engine acts as a minimal `Launcher` and `Bootstrapper` that delegates all actual logic (including core systems like logging, rendering, and gameplay) to dynamic plugins via a strict topological load graph and C-ABI boundary. The core utilizes `Flecs` for its ECS (Entity Component System), `glaze` for JSON processing, and `FlatBuffers` for zero-copy ABI-safe message passing.

## Key Architectural Principles

1. **The C-ABI Boundary Rule**
   Plugins ONLY interact with the engine and each other via stable C-ABI safe interfaces (`sandbox::api::*`). This means no C++ exceptions can propagate across DLL boundaries, and `std::` types (like vectors or strings) are never passed between the engine and plugins directly. All complex data uses FlatBuffers or lightweight C wrappers.

2. **Strict Encapsulation**
   The internal implementations (`src/subsystems`) are completely hidden. Plugins only include `sandbox/api/*` wrappers, isolating the compilation and preventing STL/ABI mismatches.

3. **Treat the Engine as a Library**
   The Sandbox core engine is statically available and treated as just another module. Core subsystems (`core_logger`, `core_vfs`, `core_runner`) are injected into the library registry exactly like plugins. This allows you to easily statically link your own plugins into the engine during debugging!

## Quick Start
Build the engine using CMake:

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

To run the engine:
```bash
./bin/sandbox --mount path/to/your/app.zip
```

If you are a developer, run the integration test suite:
```bash
./bin/sandbox_tests
```

## Plugin Development
See the [WIKI](WIKI.md) for instructions on creating plugins and writing modules using the new simplified SDK wrappers!
