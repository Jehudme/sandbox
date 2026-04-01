# MVP Milestone

This document defines the Minimum Viable Product (MVP) milestone for the Sandbox engine. The MVP represents the smallest complete set of subsystems needed for a functional, integration-testable runtime: a host application can boot an engine instance, compose extensions, publish events, and progress a Flecs world through at least one tick.

---

## Scope

### In scope

| Subsystem | Component | Description |
|-----------|-----------|-------------|
| **core** | `engine` | Bootstrap, extension lifecycle, `initialize` / `finalize` / `progress` |
| **core** | `plugins` | Abstract base for loadable plugin modules |
| **core** | `type_registry` | RTTR-backed reflection registration and lookup |
| **core** | `properties` | Hierarchical JSON configuration tree |
| **ecs** | `events` | Typed event publishing and named observer subscription |
| **ecs** | `scopes` | ECS namespace / scope management |
| **ecs** | `stages` | Custom Flecs phase creation with dependency ordering |
| **ecs** | `systems` | Named Flecs system lifecycle (create, enable, disable, destroy) |
| **ecs** | `triggers` | Named Flecs observer lifecycle (create, enable, disable, destroy) |
| **data** | `storage` | Typed ECS-backed object construction, lookup, and destruction |
| **data** | `caches` | Cache-aside entity lookup with optional TTL expiry |
| **io** | `filesystem` | Virtual-to-physical path resolution via mount tables |
| **io** | `serializer` | JSON save/load workflows for persistent entities |
| **system** | `clock` | Frame delta, fixed-step accumulation, and time-scale control |
| **system** | `dependencies` | Extension readiness gating via dependency contracts |
| **diagnostics** | `logger` | spdlog-backed structured logging wired to the event stream |
| **diagnostics** | `diagnostics` | Per-system timing metrics and FPS collection |

### Out of scope for MVP

- Rendering / windowing
- Audio
- Networking
- Asset hot-reload
- Editor tooling

---

## Acceptance Criteria

1. `engine::initialize()` creates and initializes all in-scope extensions in dependency order without errors.
2. At least one `systems` system and one `triggers` trigger can be created, enabled, disabled, and destroyed via events.
3. `clock` produces a non-zero `dt` after the first `engine::progress()` call.
4. `filesystem` resolves a mounted virtual path to a physical path.
5. `serializer` can save and reload component state for an entity tagged `persistent_tag`.
6. `storage` creates, retrieves, and destroys a typed object by name.
7. `caches` stores an entity reference and returns it on a subsequent lookup.
8. `dependencies` marks the world ready when all declared requirements are present.
9. `logger` emits an info-level log line through the event stream.
10. `diagnostics` records at least one non-zero system timing after a `progress()` call.
11. `engine::finalize()` tears down all extensions without crashes or leaks.
12. The `launcher` target builds cleanly and exits with code 0.

---

## Milestones Checklist

### Foundation (already merged into `refactoring`)
- [x] `properties` — hierarchical JSON config tree
- [x] `plugins` — abstract plugin base class
- [x] `type_registry` — RTTR reflection registry

### ECS layer
- [ ] `engine` — core bootstrap with full extension lifecycle
- [ ] `events` extension
- [ ] `scopes` extension
- [ ] `stages` extension
- [ ] `systems` extension
- [ ] `triggers` extension

### Data layer
- [ ] `storage` extension
- [ ] `caches` extension

### IO layer
- [ ] `filesystem` extension
- [ ] `serializer` extension

### System layer
- [ ] `clock` extension
- [ ] `dependencies` extension

### Diagnostics layer
- [ ] `logger` extension
- [ ] `diagnostics` extension

### Integration
- [ ] `launcher` boots engine, runs one tick, and exits cleanly
- [ ] All acceptance criteria verified
