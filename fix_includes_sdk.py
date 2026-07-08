import os
import re

def update_includes_in_dir(directory, project_name):
    # Regex to match #include <sandbox/services/foo_service.h>
    pattern = re.compile(rf'#include <{project_name}/services/([a-zA-Z0-9_]+)_service\.h>')
    
    for root, dirs, files in os.walk(directory):
        for file in files:
            if not file.endswith(('.cpp', '.h', '.hpp')):
                continue
                
            filepath = os.path.join(root, file)
            with open(filepath, 'r') as f:
                content = f.read()
                
            # Replace it with #include <project_name/sdk/foo.hpp>
            # Wait, wait, wait! If I just replace it, what about things that only want the C API?
            # E.g., the bootstrapper, module impl? 
            # In C++ code (tests, launcher, etc) using the SDK is usually what they want.
            # Let's replace ONLY if the file uses the C++ SDK.
            # C++ SDK is used as `sandbox::modules::foo::`
            
            # Or I can just replace all of them since it's C++ anyway?
            # It's safer to only replace them if they use the C++ namespace, 
            # or if it's in the tests/ or application/ directories.
            
            # Actually, the user wants the tests and application to use the SDK.
            # Let's just do a naive replacement and fix whatever breaks.
            new_content = pattern.sub(rf'#include <{project_name}/sdk/\1.hpp>', content)
            
            if new_content != content:
                with open(filepath, 'w') as f:
                    f.write(new_content)
                print(f"Updated {filepath}")

# For sandbox, tests and launcher and maybe sandbox/source/core
update_includes_in_dir('/home/jehud/CLionProjects/sandbox/tests', 'sandbox')
update_includes_in_dir('/home/jehud/CLionProjects/sandbox/launcher', 'sandbox')
update_includes_in_dir('/home/jehud/CLionProjects/sandbox/sandbox/source', 'sandbox')

# For spectre, tests
update_includes_in_dir('/home/jehud/CLionProjects/spectre/tests', 'spectre')
