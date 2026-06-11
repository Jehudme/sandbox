# 7. ABI Safety Concepts

*This reference lists the strict C++ type traits and ABI boundary rules required when building Sandbox Engine plugins.*

## Component Safety Traits (Flecs Constraints)
When registering a struct as an Entity Component System (ECS) component, it **must** satisfy specific C++ type traits. If it does not, you risk triggering Undefined Behavior (UB) across compilers.

1.  **`std::is_trivially_copyable_v<T>` == true**
    Flecs moves and copies memory using `memcpy()`. If your component has a custom copy constructor or a non-trivial destructor (like a `std::string` allocating heap memory), it will leak or crash.
2.  **`std::is_standard_layout_v<T>` == true**
    The struct must have the same memory layout across GCC, MSVC, and Clang. Do not use virtual functions, base classes with their own fields, or mixed access specifiers in the struct.

## The Raw C-ABI Service Structure
The engine passes services as C-structs of function pointers.

```cpp
struct logger_service {
    void* instance; // Opaque pointer hiding the C++ object implementation
    int32_t (*log)(void* instance, const uint8_t* log_msg_fb, size_t size);
};
```
*   **The Return Type Rule:** Functions crossing the DLL wall must return an integer code (`int32_t`). Returning C++ objects (like `std::expected` or `std::string`) relies on ABI-specific registers and will cause stack corruption.
*   **The Instance Pointer:** C++ member functions implicitly take a `this` pointer. The C-ABI struct mimics this by explicitly passing `void* instance` as the first argument to every function pointer.

## The Exception Firewall
*   **Rule:** NO exceptions may cross the `.so` / `.dll` boundary.
*   **Enforcement:** All SDK wrappers wrap `try/catch` internally if they interact with Glaze, and map integer return codes to `std::unexpected` errors.

---

# 8. FlatBuffer Schemas

*Reference list of the standard FlatBuffer payloads used by the engine's internal services.*

## `logger.fbs`
Used by the `logger_service` to transfer log requests.
```flatbuffers
namespace sandbox::schemas::logger;

enum LogLevel : byte { Trace = 0, Debug, Info, Warn, Error, Fatal }

table LogMessage {
  level: LogLevel = Info;
  message: string (required);
  source_file: string;
  source_line: int;
  throw_on_error: bool = false;
}

root_type LogMessage;
```

## `filesystem.fbs`
Used by the `filesystem_service` to return complex data like directory listings or file metadata.
```flatbuffers
namespace sandbox::schemas;

table StringList {
  items: [string];
}

enum FileType : byte { File = 0, Directory, Symlink, Other }

table FileMetadata {
  size: uint64;
  type: FileType;
  modification_time: uint64;
}
```
