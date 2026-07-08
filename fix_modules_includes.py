import os
import re

def update_includes_in_dir(directory, project_name):
    pattern = re.compile(rf'#include <{project_name}/services/([a-zA-Z0-9_]+)_service\.h>')
    
    for root, dirs, files in os.walk(directory):
        for file in files:
            if not file.endswith(('.cpp', '.h', '.hpp')):
                continue
                
            filepath = os.path.join(root, file)
            with open(filepath, 'r') as f:
                content = f.read()
                
            new_content = pattern.sub(rf'#include <{project_name}/sdk/\1.hpp>', content)
            
            if new_content != content:
                with open(filepath, 'w') as f:
                    f.write(new_content)
                print(f"Updated {filepath}")

update_includes_in_dir('/home/jehud/CLionProjects/sandbox/modules/core/source', 'sandbox')
