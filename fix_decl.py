import os
import re

core_dir = '/home/jehud/CLionProjects/sandbox/modules/core/source'
spectre_dir = '/home/jehud/CLionProjects/spectre/spectre/source/modules'

def replace_in_files(directory):
    for root, _, files in os.walk(directory):
        for file in files:
            if file.endswith('_module.cpp'):
                path = os.path.join(root, file)
                with open(path, 'r') as f:
                    content = f.read()
                
                new_content = re.sub(r'\.service = &sandbox_([a-z_]+)_service_t_info_decl,', r'.service = &sandbox_\1_service_t_info,', content)
                new_content = re.sub(r'\.service = &spectre_([a-z_]+)_service_t_info_decl,', r'.service = &spectre_\1_service_t_info,', new_content)
                
                if new_content != content:
                    with open(path, 'w') as f:
                        f.write(new_content)
                    print(f"Updated {path}")

replace_in_files(core_dir)
replace_in_files(spectre_dir)
