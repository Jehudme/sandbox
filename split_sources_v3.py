import os
import re

core_dir = '/home/jehud/CLionProjects/sandbox/modules/core'

jobs = [
    ('application', 'application', 30, 42),
    ('configuration', 'configuration', 31, 53),
    ('filesystem', 'filesystem', 294, 675),
    ('logs', 'logger', 142, 195),
    ('runtime', 'runtime', 66, 121)
]

for mod, basename, start_line, end_line in jobs:
    mod_dir = f'{core_dir}/source/{mod}'
    cpp_file = f'{mod_dir}/{basename}.cpp'
    h_file = f'{mod_dir}/{basename}.h'
    
    with open(cpp_file, 'r') as f:
        lines = f.readlines()
        
    abi_lines = lines[start_line-1 : end_line-1]
    module_lines = lines[:start_line-1] + lines[end_line-1:]
    
    abi_content = "".join(abi_lines)
    
    # Strip includes from abi_content
    abi_content = re.sub(r'^#include .*$\n', '', abi_content, flags=re.MULTILINE)
    
    # Clean up empty lines at start
    abi_content = abi_content.lstrip()
    
    # Replace variable names
    abi_content = abi_content.replace('ecs_world_t* ecs', 'ecs_world_t* entity_world')
    abi_content = abi_content.replace('ecs_world_t* w', 'ecs_world_t* entity_world')
    abi_content = abi_content.replace('if (!ecs)', 'if (!entity_world)')
    abi_content = abi_content.replace('(void)ecs;', '(void)entity_world;')
    abi_content = abi_content.replace('flecs::world w(ecs);', 'flecs::world flecs_world(entity_world);')
    abi_content = abi_content.replace('w.try_get_mut', 'flecs_world.try_get_mut')
    abi_content = abi_content.replace('w.get_mut', 'flecs_world.get_mut')
    abi_content = abi_content.replace('w.has', 'flecs_world.has')
    abi_content = abi_content.replace('auto* m =', 'auto* module =')
    abi_content = abi_content.replace('if (m)', 'if (module)')
    abi_content = abi_content.replace('m->', 'module->')
    
    service_cpp = f"""#include "sandbox/services/{mod}_service.h"
#include "{mod}_module.h"
#include <flecs.h>

{abi_content.strip()}
"""
    
    with open(f'{mod_dir}/{mod}_service.cpp', 'w') as f:
        f.write(service_cpp)
        
    cpp_content = "".join(module_lines)
    
    cpp_content = cpp_content.replace(f'#include "{basename}.h"', f'#include "{mod}_module.h"')
    
    with open(f'{mod_dir}/{mod}_module.cpp', 'w') as f:
        f.write(cpp_content)
        
    os.rename(h_file, f'{mod_dir}/{mod}_module.h')
    
    os.remove(cpp_file)
    
    print(f"Processed {mod}")
