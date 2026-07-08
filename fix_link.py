import os
import re

def move_impl_to_header(h_file, cpp_file):
    if not os.path.exists(h_file): return
    with open(h_file, 'r') as f:
        h_content = f.read()
    
    if cpp_file and os.path.exists(cpp_file):
        with open(cpp_file, 'r') as f:
            cpp_content = f.read()
    else:
        cpp_content = ""

    # Find the implementations block in cpp
    impl_pattern = re.compile(r'// --- Public C API Implementations ---\n(.*?)$', re.DOTALL)
    m = impl_pattern.search(cpp_content)
    if not m:
        return
        
    impls = m.group(1).strip()
    
    # Remove from CPP
    cpp_content = impl_pattern.sub('', cpp_content).strip() + '\n'
    with open(cpp_file, 'w') as f:
        f.write(cpp_content)
        
    # In header, replace the declarations with static inline implementations
    # The header has // --- Public C API --- followed by decls until typedef struct
    decl_pattern = re.compile(r'// --- Public C API ---\n.*?\n\ntypedef struct', re.DOTALL)
    
    # We need to make the implementations static inline in C and C++
    # Since they use C++ code (flecs::world flecs_world(world)), they can ONLY be compiled in C++ mode!
    # Wait, if they use `flecs::world`, they are NOT pure C functions! They require <flecs.hpp>!
    # If they require <flecs.hpp>, they must be guarded by #ifdef __cplusplus
    
    # Actually, the user's example was:
    # bool is_running(const flecs::world& entity_world) {
    #     if (const auto* service = SANDBOX_GET_SERVICE(entity_world, sandbox_application_service_t)) { ...
    
    # But I wrote the C wrappers taking `ecs_world_t* world`.
    # `flecs::world flecs_world(world);` is C++ code!
    pass

