import os
import re

def fix_cpp_file(filepath):
    with open(filepath, 'r') as f:
        content = f.read()

    # Find the pattern: if (const auto* service = SANDBOX_GET_SERVICE(var_name, service_name)) {
    pattern = re.compile(r'if\s*\(\s*const\s+auto\s*\*\s*service\s*=\s*SANDBOX_GET_SERVICE\s*\(\s*([^,]+)\s*,\s*(\w+_service_t)\s*\)\s*\)\s*\{')
    
    def repl(m):
        var_name = m.group(1)
        service_name = m.group(2)
        return f'flecs::world flecs_world({var_name});\n    if (const auto* service = SANDBOX_GET_SERVICE(flecs_world, {service_name})) {{'
        
    new_content = pattern.sub(repl, content)
    
    if new_content != content:
        with open(filepath, 'w') as f:
            f.write(new_content)
        print(f"Fixed {filepath}")

def run_sandbox():
    base = '/home/jehud/CLionProjects/sandbox/modules/core/source'
    for root, dirs, files in os.walk(base):
        for file in files:
            if file.endswith('_service.cpp'):
                fix_cpp_file(os.path.join(root, file))

def run_spectre():
    base = '/home/jehud/CLionProjects/spectre/spectre/source/modules'
    for root, dirs, files in os.walk(base):
        for file in files:
            if file.endswith('_service.cpp'):
                fix_cpp_file(os.path.join(root, file))

run_sandbox()
run_spectre()
