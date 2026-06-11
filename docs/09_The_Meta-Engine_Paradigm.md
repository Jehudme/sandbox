# 9. The Meta-Engine Paradigm & C-ABI Boundary

*This explanation details why the engine is architected as an isolated graph of DLLs and the mathematical necessity of the ABI wall.*

## The Meta-Engine Concept
Unlike traditional monolith engines (like Unreal or Godot), the Sandbox Engine is a "Meta-Engine". The base executable (`sandbox`) is virtually empty. It is merely a host loop. Every major piece of functionality—physics, rendering, input, and even the core Logger and Virtual File System—are implemented as isolated dynamic libraries (`.dll` / `.so`).

This allows game developers to swap out the entire rendering core simply by editing a JSON manifest, without ever recompiling the engine.

## The C-ABI Boundary
When plugins are compiled dynamically, they suffer from the **Application Binary Interface (ABI) mismatch problem**.

### The Problem
If Plugin A is compiled with MSVC 19.30 and Plugin B is compiled with Clang 15, their internal implementation of `std::string` differs. 
If Plugin A calls a function in Plugin B and hands it a `std::string`, Plugin B might read the memory expecting a 24-byte layout, while Plugin A passed a 32-byte layout. 

**Result:** Immediate segmentation fault or silent memory corruption.

### The Solution: The Wall
The engine solves this by building a massive "wall" at the DLL boundary. 
No C++ Standard Library types (`std::string`, `std::vector`, `std::shared_ptr`) are permitted to cross this wall.

Instead, the plugins communicate using **Pure C Structs**.
```cpp
// A pure C Struct is mathematically identical in memory across all C/C++ compilers
struct raw_vector3 {
    float x;
    float y;
    float z;
};
```
When you use the `sandbox::sdk` wrappers, the wrapper serializes your modern C++ object (like a `std::string`) into a raw byte array, casts it to an opaque C-pointer, throws it over the wall to the other plugin, and the other plugin casts it back to its own local representation of a string.

## Exceptions Are Illegal
If a plugin throws a `std::runtime_error` and the caller is inside a different DLL, the stack unwinding process will cross the ABI boundary. Different compilers use entirely different mechanisms for stack unwinding (SJLJ vs DWARF). This will cause `std::terminate` to be called, killing the engine instantly.

Therefore, all functions crossing the C-ABI boundary return integer error codes (`0` for success). The C++ SDK wrappers intercept these integer codes and elevate them safely into `std::expected` types locally.
