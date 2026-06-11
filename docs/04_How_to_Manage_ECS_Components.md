# 4. How to Create and Manage ECS Components

*This practical guide explains the strict memory rules for Flecs components and how to use the Handle Pattern for heavy C++ objects.*

**Conclusion first:** Flecs components must be Plain Old Data (POD) to ensure cache locality and ABI safety. You cannot put STL containers (`std::vector`, `std::string`) or virtual classes inside a component. To manage heavy objects, you must store them in a private array and pass a numeric ID (a Handle) to the ECS component.

## The Plain Old Data (POD) Rule
When you register a component in Flecs, Flecs allocates dense contiguous arrays of memory. If a component contains a pointer to a heap allocation (like a `std::string`), passing it between plugins compiled with different compilers will cause segmentation faults due to mismatched destructor implementations.

### The BAD Component (Will Crash)
```cpp
//  DO NOT DO THIS
struct enemy_data {
    std::string name; // Different compilers implement std::string differently (SSO)
    std::vector<int> pathing_nodes; // Flecs will not call the destructor! Memory leak!
    virtual void attack() {} // vtables break C-ABI compatibility completely
};
```

### The GOOD Component (ABI-Safe)
```cpp
//  DO THIS
struct enemy_data {
    uint32_t model_handle; // Handle ID
    float health;
    float position[3]; // Raw array, totally fine
    bool is_aggro;
};
```
If you define `enemy_data` like this, it is trivially copyable (`std::is_trivially_copyable_v<enemy_data> == true`). It is safe across all boundaries.

## The Handle Pattern
If you need to associate an entity with an open File Stream or a heavy Vulkan Texture, you must use the **Handle Pattern**. 

1. Your plugin owns a private `std::vector` or `std::unordered_map` that holds the heavy objects.
2. The ECS component stores the integer index (Handle) to that object.

### Implementation Example

```cpp
#include <flecs.h>
#include <unordered_map>
#include <memory>

// 1. The heavy C++ object (Private to the plugin)
struct heavy_texture {
    std::vector<uint8_t> pixels;
    int width, height;
    ~heavy_texture() { /* Complex cleanup */ }
};

// 2. The Plugin Module Logic
struct rendering_module {
    // Private heap map owned exclusively by the plugin
    std::unordered_map<uint32_t, std::unique_ptr<heavy_texture>> m_textures;
    uint32_t m_next_handle = 1;

    // 3. The POD ECS Component
    struct texture_component {
        uint32_t handle_id;
    };

    rendering_module(flecs::world& ecs) {
        ecs.component<texture_component>();

        // 4. Create an entity with a handle
        uint32_t new_id = m_next_handle++;
        m_textures[new_id] = std::make_unique<heavy_texture>();
        
        ecs.entity("PlayerSprite").set<texture_component>({new_id});

        // 5. Resolve the handle in a system
        ecs.system<const texture_component>("RenderSystem")
            .each([this](flecs::entity e, const texture_component& tex) {
                // Look up the heavy object safely using the handle
                auto it = m_textures.find(tex.handle_id);
                if (it != m_textures.end()) {
                    heavy_texture* tex_ptr = it->second.get();
                    // Draw texture...
                }
            });
    }
};
```
This guarantees the plugin allocates and deletes its own memory safely without polluting the ECS!
