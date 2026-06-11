# Explanation: Kahn's Sort & The Bootstrapper

When the engine starts, it reads the `manifest.json` to discover all required plugins. However, plugins have dependencies. `Renderer` might depend on `Window`, which depends on `Logger`.

## Topological Sorting
The Bootstrapper uses **Kahn's Algorithm** to perform a topological sort on the dependency graph.

1. It calculates the "in-degree" (number of incoming dependencies) for every module.
2. It finds all modules with an in-degree of 0 (no dependencies) and pushes them to a queue.
3. It pops a module, activates it, and reduces the in-degree of all modules that depended on it.
4. If a module's in-degree hits 0, it is added to the queue.

This guarantees that a module is *never* loaded before its dependencies are fully initialized and registered in the ECS.

## Circular Dependencies
If Kahn's Algorithm completes but there are still modules left with an in-degree greater than 0, the Bootstrapper immediately halts and throws a `boot_error`. This prevents infinite loops caused by circular dependencies.
