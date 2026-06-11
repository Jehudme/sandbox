# How-To: Read and Write to the Virtual File System

PhysicsFS provides a jailed Virtual File System (VFS). All file access must occur through `mount://` URIs.

## Reading a File

Use the SDK wrapper to ensure ABI safety.

```cpp
#include <sandbox/modules/filesystem/filesystem_api.h>
#include <iostream>

void read_config(flecs::world& ecs) {
    sandbox::sdk::filesystem fs(ecs);
    
    auto res = fs.read_text("mount://app/config.json");
    if (res.has_value()) {
        std::cout << "Config: " << res.value() << std::endl;
    } else {
        std::cerr << "Error: " << res.error() << std::endl;
    }
}
```

## Writing a File

```cpp
#include <sandbox/modules/filesystem/filesystem_api.h>

void write_save(flecs::world& ecs, const std::string& data) {
    sandbox::sdk::filesystem fs(ecs);
    
    auto res = fs.write("mount://cache/save1.dat", data);
    if (!res.has_value()) {
        std::cerr << "Failed to save: " << res.error() << std::endl;
    }
}
```
