# Explanation: The Meta-Engine Paradigm

The Sandbox Engine is fundamentally a **Meta-Engine**. It is an engine designed to build other engines.

## The Problem
Traditional game engines are monolithic. If you want to replace the renderer, the physics system, or the scripting language, you must fork the engine and recompile it. This leads to rigid architectures.

## The Solution
The Sandbox Engine acts purely as a `Bootstrapper` and a `Registry`. It provides zero gameplay logic, zero rendering, and zero physics. Instead, it provides a strictly ordered DAG (Directed Acyclic Graph) of dynamic libraries (`.dll` / `.so`).

Every subsystem—even the core logger and virtual filesystem—is a plugin. This allows developers to hot-swap out the core systems by simply modifying the `manifest.json`.

If an application needs an Unreal-style architecture, it loads modules that establish that architecture. If it needs a minimalist 2D framework, it loads entirely different modules. The engine is a blank slate dictated entirely by the loaded plugins.
