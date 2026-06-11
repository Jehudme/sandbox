# 11. Kahn's Topological Sort

*This explanation deep-dives into how the Bootstrapper mathematically resolves the plugin dependency graph safely.*

## The Problem with Dynamic Loading
If you have 5 plugins: `Renderer`, `Physics`, `Audio`, `PlayerMovement`, and `Logger`.
If `Renderer` needs the `Logger` to print its boot sequence, `Logger` MUST be loaded first. If they load in alphabetical order (`Audio` -> `Logger` -> `Physics` -> `PlayerMovement` -> `Renderer`), `Renderer` boots fine.
But if `Audio` needs `Physics`, it crashes!

## Directed Acyclic Graphs (DAGs)
When a plugin uses the `SANDBOX_DECLARE_MODULE` macro, it explicitly lists its dependencies. The Bootstrapper (`bootstrapper.cpp`) takes all these declarations and builds a mathematical Graph.

*   Each Plugin is a Node.
*   Each Requirement is a directed Edge.

### Kahn's Algorithm
The bootstrapper applies Kahn's Algorithm for Topological Sorting to figure out the exact correct order to call the constructors for the plugins.

1.  **Calculate In-Degree:** The bootstrapper counts how many incoming edges (dependencies) a node has. `Logger` has 0. `Physics` requires `Logger` (in-degree 1). `Audio` requires `Logger` and `Physics` (in-degree 2).
2.  **Queue Zero-Degrees:** It finds all nodes with an in-degree of 0 (e.g., `Logger`) and puts them in a Queue.
3.  **Process:** It pops `Logger` from the queue, calls its constructor (booting it into the ECS), and then "deletes" the outgoing edges from `Logger`.
4.  **Decrement:** Because the edge from `Logger` to `Physics` is deleted, `Physics`'s in-degree drops from 1 to 0. 
5.  **Re-queue:** Since `Physics` is now 0, it goes into the Queue!

### Cyclic Graph Detection
What happens if a junior developer makes a mistake?
*   `Physics` requires `Renderer`.
*   `Renderer` requires `Physics`.

Kahn's Algorithm will process `Logger`, but then `Physics` and `Renderer` will both be stuck at an in-degree of 1. The Queue will be empty.
The bootstrapper detects that the number of booted modules is less than the total number of requested modules. It immediately realizes a **Circular Dependency** exists and fatally aborts the engine launch, printing exactly which modules are locked in the infinite loop. This completely protects the engine from infinite boot deadlocks.
