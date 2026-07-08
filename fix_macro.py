import os
import re

def fix_macro(h_file):
    with open(h_file, 'r') as f:
        content = f.read()

    # We just need to replace SANDBOX_GET_SERVICE(world_arg, prefix_service_t) with ecs_singleton_get(world_arg, prefix_service_t)
    # in the // --- Public C API --- block
    
    api_block_pattern = re.compile(r'(// --- Public C API ---\n.*?)\n\n#ifdef __cplusplus', re.DOTALL)
    m = api_block_pattern.search(content)
    if not m:
        return
        
    api_block = m.group(1)
    
    # Replace SANDBOX_GET_SERVICE with ecs_singleton_get
    new_api_block = re.sub(r'SANDBOX_GET_SERVICE\s*\(([^,]+),\s*(\w+)\)', r'ecs_singleton_get(\1, \2)', api_block)
    
    if new_api_block != api_block:
        new_content = content.replace(api_block, new_api_block)
        with open(h_file, 'w') as f:
            f.write(new_content)
        print(f"Fixed {h_file}")

def run():
    s_base = '/home/jehud/CLionProjects/sandbox/modules/core/include/sandbox/services'
    for f in os.listdir(s_base):
        if f.endswith('_service.h'):
            fix_macro(os.path.join(s_base, f))

    sp_base = '/home/jehud/CLionProjects/spectre/spectre/include/spectre/services'
    for f in os.listdir(sp_base):
        if f.endswith('_service.h'):
            fix_macro(os.path.join(sp_base, f))

run()
