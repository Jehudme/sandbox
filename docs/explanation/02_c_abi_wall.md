# Explanation: The C-ABI Firewall

In C++, passing objects across DLL boundaries is catastrophically dangerous.

## The DLL Boundary Problem
If `Plugin A` allocates a `std::vector` and passes it to `Plugin B`, and `Plugin B` modifies or frees it, the program will likely crash. This is because:
1. `Plugin A` and `Plugin B` might be compiled with different heap allocators.
2. They might use different standard libraries (e.g., MSVC vs. libstdc++).
3. They might be compiled with different C++ versions (C++20 vs C++23).
4. Exceptions thrown in `Plugin A` cannot be safely caught in `Plugin B`.

## The Sandbox Solution: The C-ABI Wall
The Sandbox Engine forces all communication between plugins to occur over a strict **C-ABI Firewall**.

All services must be defined as C-structs containing a single function pointer: `execute_command`.

```cpp
struct my_service {
    void* instance;
    void (*execute_command)(void* instance, uint32_t command, const uint8_t* payload, size_t size);
};
```

This enforces that only primitive integers, raw memory pointers, and flat byte arrays (FlatBuffers) can cross the boundary. 

To maintain modern C++ ergonomics, we use **SDK Wrappers** (`sandbox::sdk::*`) that internally serialize the arguments into FlatBuffers, invoke the C-ABI function, and deserialize the result into `std::expected`. This gives you the safety of C and the elegance of C++23.
