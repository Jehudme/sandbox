import os
import re

core_dir = '/home/jehud/CLionProjects/sandbox/modules/core/source'

for root, _, files in os.walk(core_dir):
    for file in files:
        if file.endswith('_module.cpp'):
            path = os.path.join(root, file)
            with open(path, 'r') as f:
                content = f.read()
            
            content = re.sub(r'\.service = &sandbox_([a-z_]+)_service_t_info,', r'.service = &sandbox_\1_service_t_info_decl,', content)
            
            with open(path, 'w') as f:
                f.write(content)
                
# Fix logs_service.cpp missing spdlog include
logs_svc = f'{core_dir}/logs/logs_service.cpp'
with open(logs_svc, 'r') as f:
    content = f.read()
if '<spdlog/spdlog.h>' not in content:
    content = content.replace('#include "logs_module.h"', '#include "logs_module.h"\n#include <spdlog/spdlog.h>')
with open(logs_svc, 'w') as f:
    f.write(content)
    
# Fix filesystem_service.cpp missing logs_service include
fs_svc = f'{core_dir}/filesystem/filesystem_service.cpp'
with open(fs_svc, 'r') as f:
    content = f.read()
if 'logs_service.h' not in content:
    content = content.replace('#include "filesystem_module.h"', '#include "filesystem_module.h"\n#include <sandbox/services/logs_service.h>')
with open(fs_svc, 'w') as f:
    f.write(content)

print("Fixes applied.")
