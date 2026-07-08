import os
import re

def fix_header_order(h_file):
    with open(h_file, 'r') as f:
        content = f.read()

    # We want to move the // --- Public C API --- block from its current location to right before the closing #ifdef __cplusplus
    
    api_block_pattern = re.compile(r'(// --- Public C API ---\n.*?)\n\ntypedef struct', re.DOTALL)
    m = api_block_pattern.search(content)
    if not m:
        return
        
    api_block = m.group(1)
    
    # Remove the api block from where it is
    new_content = content.replace(api_block + '\n\n', '')
    
    # Insert it right before the last #ifdef __cplusplus
    insert_pattern = re.compile(r'(#ifdef __cplusplus\s*\}\s*#endif\s*)$')
    if insert_pattern.search(new_content):
        new_content = insert_pattern.sub(f'{api_block}\n\n\\1', new_content)
    else:
        new_content += f'\n{api_block}\n'
        
    with open(h_file, 'w') as f:
        f.write(new_content)

def run():
    s_base = '/home/jehud/CLionProjects/sandbox/modules/core/include/sandbox/services'
    for f in os.listdir(s_base):
        if f.endswith('_service.h'):
            fix_header_order(os.path.join(s_base, f))

    sp_base = '/home/jehud/CLionProjects/spectre/spectre/include/spectre/services'
    for f in os.listdir(sp_base):
        if f.endswith('_service.h'):
            fix_header_order(os.path.join(sp_base, f))

run()
