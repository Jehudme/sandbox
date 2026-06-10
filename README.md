# Sandbox Meta-Engine

Welcome to the **Sandbox Meta-Engine**, a cutting-edge C++23 Dynamic Meta-Engine Framework built from the ground up for absolute ABI safety across dynamic module boundaries. By enforcing a rigid architectural divide, the engine achieves robust modularity, allowing developers to hot-swap plugins without fear of ABI breakages, memory boundary crashes, or compiler fragmentation.

## Core Features

- **Strict C-ABI Boundary**: Using the Hourglass Pattern, plugins never see the engine's internal STL states (`std::vector`, `std::string`, `std::expected`). Instead, all data across the DLL boundary is safely packed into raw memory blocks and FlatBuffers, ensuring memory safety and binary compatibility across compiler versions.
- **Topological Module Loader**: Powered by Kahn's Algorithm, the `bootstrapper` natively resolves complex dependency graphs for plugins and services at runtime. Ensure your modules boot safely exactly when their dependencies are ready!
- **Flecs Integration**: Utilizing [Flecs](https://github.com/SanderMertens/flecs), the engine natively drives entity-component-system (ECS) logic. The ECS world is safely shared with plugins to serve as the unified registry and dependency injector.
- **Virtual Filesystem (VFS)**: Mount diverse physical directories or archives into a unified, secure virtual tree natively powered by PhysFS, all accessible seamlessly via the SDK wrappers.
- **Modern C++23 SDK Wrappers**: Plugin developers write beautiful, idiomatic C++23. The SDK wrappers instantly convert your modern vectors and expected results into FlatBuffer payloads for safe ABI transit!

## Directory Structure Overview

The engine enforces an uncompromising physical boundary between internal implementations and public-facing interfaces:

- **`include/` (Public API)**: Contains ONLY the SDK and safe Engine headers. Plugin developers build exclusively against this folder (`api/` wrappers and `core/` orchestration headers). The hidden internal interfaces are strictly invisible.
- **`src/` (Private Engine)**: The core implementations, private subsystem interfaces (`ifilesystem.h`, `ilogger.h`), FlatBuffers schemas, and generated headers live exclusively here. When the engine builds, it merges `src` and `include`, but plugins are never granted access to `src`.

## Building the Engine

### Prerequisites
- CMake 3.20+
- A C++23 compliant compiler (GCC 13+, Clang 16+, MSVC 19.38+)

### Build Instructions
```bash
# Clone the repository
git clone https://github.com/your-org/sandbox-engine.git
cd sandbox-engine

# Generate the CMake build system
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release

# Build the engine, launcher, and tests
cmake --build build

# Run the integration tests
./build/bin/sandbox_tests

# Run the launcher
./build/bin/launcher -m /path/to/mount
```

## Creating Plugins
Want to create a plugin? The engine makes it effortless! By using `SANDBOX_DECLARE_MODULE` and the engine's Flecs integration, you can build dynamic shared libraries that hook safely into the meta-engine.

See the [WIKI.md](WIKI.md) for the complete Plugin Developer Guide.
