import os

core_dir = '/home/jehud/CLionProjects/sandbox/modules/core/source'
modules = ['application', 'configuration', 'filesystem', 'logs', 'runtime']

for mod in modules:
    cpp_path = f'{core_dir}/{mod}/{mod}_module.cpp'
    if not os.path.exists(cpp_path):
        continue
    with open(cpp_path, 'r') as f:
        content = f.read()
        
    include_str = f'#include <sandbox/services/{mod}_service.h>'
    if include_str not in content:
        # Insert after the module.h include
        mod_h = f'#include "{mod}_module.h"\n'
        if mod_h in content:
            content = content.replace(mod_h, f'{mod_h}{include_str}\n')
        else:
            content = f'{include_str}\n{content}'
            
    with open(cpp_path, 'w') as f:
        f.write(content)
        
# Fix cstring in filesystem_service.cpp
fs_svc = f'{core_dir}/filesystem/filesystem_service.cpp'
with open(fs_svc, 'r') as f:
    content = f.read()
if '<cstring>' not in content:
    content = content.replace('#include "filesystem_module.h"', '#include "filesystem_module.h"\n#include <cstring>')
with open(fs_svc, 'w') as f:
    f.write(content)
