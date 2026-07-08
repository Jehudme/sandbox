import os
import re

modules = ['application', 'configuration', 'filesystem', 'logs', 'runtime']

def replace_includes_in_file(filepath):
    with open(filepath, 'r') as f:
        content = f.read()
    
    original_content = content
    
    for mod in modules:
        content = re.sub(rf'#include ["<]sandbox/sdk/{mod}\.hpp[">]', f'#include <sandbox/services/{mod}_service.h>', content)
        content = re.sub(rf'#include ["<]sandbox/abi/{mod}\.h[">]', f'#include <sandbox/services/{mod}_service.h>', content)
        
    if content != original_content:
        with open(filepath, 'w') as f:
            f.write(content)
        print(f"Updated {filepath}")

for root, dirs, files in os.walk('/home/jehud/CLionProjects/sandbox'):
    if '.git' in root or 'build' in root or 'cmake-build' in root:
        continue
    for file in files:
        if file.endswith(('.h', '.hpp', '.c', '.cpp')):
            replace_includes_in_file(os.path.join(root, file))

for root, dirs, files in os.walk('/home/jehud/CLionProjects/spectre'):
    if '.git' in root or 'build' in root or 'cmake-build' in root:
        continue
    for file in files:
        if file.endswith(('.h', '.hpp', '.c', '.cpp')):
            replace_includes_in_file(os.path.join(root, file))

print("Include updates complete.")
