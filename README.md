# Sandbox

Sandbox is a **C++23 event-driven meta-engine** built on top of **Flecs (ECS)** and **RTTR (reflection)**. It is designed as a foundational runtime layer for specialized game engines, with extension-driven subsystems and explicit event/command orchestration.

## Build Instructions

### Requirements

- CMake 3.25+
- A C++23-capable compiler
- Git submodule dependencies (if not already present in your environment)

### Configure

```cmake
cmake -S . -B build
```

### Build launcher target

```cmake
cmake --build build --target launcher
```

### Run

```bash
./build/launcher/launcher
```

## Subsystem Architecture

Sandbox is organized into layered subsystem categories:

- **core**: engine bootstrap, extension lifecycle, and configuration properties
- **ecs**: stage/system/trigger/event/scoping wrappers over Flecs primitives
- **data**: runtime object storage, cache coordination, and reflection registry glue
- **io**: virtual filesystem path resolution and serializer orchestration
- **system**: runtime services such as timing and dependency readiness validation
- **diagnostics**: structured logging and runtime profiling metrics

Subsystem behavior is extension-centric and event-driven, enabling composition without tight compile-time coupling between services.

## Extension Documentation

Extension-specific wiki pages live in [`docs/`](./docs):

- [`docs/ecs_extensions.md`](./docs/ecs_extensions.md)
- [`docs/data_extensions.md`](./docs/data_extensions.md)
- [`docs/io_extensions.md`](./docs/io_extensions.md)
- [`docs/system_extensions.md`](./docs/system_extensions.md)
- [`docs/diagnostics_extensions.md`](./docs/diagnostics_extensions.md)
