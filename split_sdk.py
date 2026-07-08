import os
import re

def process_repo(services_dir, sdk_dir, project_name):
    if not os.path.exists(sdk_dir):
        os.makedirs(sdk_dir)
        
    for filename in os.listdir(services_dir):
        if not filename.endswith('_service.h'):
            continue
            
        module_name = filename.replace('_service.h', '')
        service_path = os.path.join(services_dir, filename)
        
        with open(service_path, 'r') as f:
            content = f.read()
            
        # The C++ block usually starts with #ifdef __cplusplus and namespace
        # Let's match from the second #ifdef __cplusplus to the end
        # Since the first one is for extern "C"
        
        # We can split by "#ifdef __cplusplus\nnamespace" or similar
        pattern = r"(?s)(#ifdef __cplusplus\s+namespace\s+" + project_name + r"::modules\s*\{.*?#endif\s*$)"
        match = re.search(pattern, content)
        
        if match:
            cpp_block = match.group(1)
            # Remove the C++ block from service header
            new_service_content = content.replace(cpp_block, '')
            
            with open(service_path, 'w') as f:
                f.write(new_service_content.strip() + '\n')
                
            # Create SDK header
            sdk_filename = f"{module_name}.hpp"
            sdk_path = os.path.join(sdk_dir, sdk_filename)
            
            # The SDK header needs to include the service header
            sdk_content = f"""#pragma once
#include <{project_name}/services/{filename}>

{cpp_block}
"""
            with open(sdk_path, 'w') as f:
                f.write(sdk_content)
            print(f"Created {sdk_path} and updated {service_path}")
        else:
            print(f"No C++ block found in {service_path}")

sandbox_services = '/home/jehud/CLionProjects/sandbox/modules/core/include/sandbox/services'
sandbox_sdk = '/home/jehud/CLionProjects/sandbox/modules/core/include/sandbox/sdk'
process_repo(sandbox_services, sandbox_sdk, 'sandbox')

spectre_services = '/home/jehud/CLionProjects/spectre/spectre/include/spectre/services'
spectre_sdk = '/home/jehud/CLionProjects/spectre/spectre/include/spectre/sdk'
process_repo(spectre_services, spectre_sdk, 'spectre')
