import os
import re

modules = ['application', 'configuration', 'filesystem', 'logs', 'runtime']
core_dir = '/home/jehud/CLionProjects/sandbox/modules/core'
include_dir = f'{core_dir}/include/sandbox'
services_dir = f'{include_dir}/services'

os.makedirs(services_dir, exist_ok=True)

for mod in modules:
    abi_path = f'{include_dir}/abi/{mod}.h'
    sdk_path = f'{include_dir}/sdk/{mod}.hpp'
    service_path = f'{services_dir}/{mod}_service.h'
    
    with open(abi_path, 'r') as f:
        abi_content = f.read()
        
    with open(sdk_path, 'r') as f:
        sdk_content = f.read()
        
    sdk_content = re.sub(r'#pragma once\n*', '', sdk_content)
    sdk_content = re.sub(rf'#include <sandbox/abi/{mod}\.h>\n*', '', sdk_content)
    
    includes = re.findall(r'^#include .*$', sdk_content, re.MULTILINE)
    sdk_content = re.sub(r'^#include .*$\n*', '', sdk_content, flags=re.MULTILINE)
    
    lines = abi_content.split('\n')
    for i, line in enumerate(lines):
        if line.startswith('#pragma once'):
            for inc in includes:
                if inc not in abi_content:
                    lines.insert(i + 1, inc)
            break
            
    abi_content = '\n'.join(lines)
    
    service_content = f"""{abi_content.strip()}

#ifdef __cplusplus
{sdk_content.strip()}
#endif
"""
    with open(service_path, 'w') as f:
        f.write(service_content)
    print(f"Generated {service_path}")
