import os
import re

def extract_braces(text, start_idx):
    count = 0
    for i in range(start_idx, len(text)):
        if text[i] == '{': count += 1
        elif text[i] == '}':
            count -= 1
            if count == 0:
                return i
    return -1

def remove_default_args(signature):
    # This regex is a simple heuristic to remove `= default_val`
    # It removes anything from '=' up to the next ',' or ')'
    return re.sub(r'\s*=[^,)]*', '', signature)

def process_file(h_file, cpp_file, sdk_file, prefix, mod):
    # 1. C API
    with open(h_file, 'r') as f: h_content = f.read()
    
    c_api_pattern = re.compile(r'static inline (.*?)\s+(\w+)\((.*?)\)\s*\{')
    
    defs = []
    
    while True:
        m = c_api_pattern.search(h_content)
        if not m: break
        
        start = m.start()
        open_brace = m.end() - 1
        close_brace = extract_braces(h_content, open_brace)
        if close_brace == -1: break
        
        ret = m.group(1)
        name = m.group(2)
        args = m.group(3)
        
        body = h_content[start:close_brace+1]
        
        macro = 'SANDBOX_API' if prefix == 'sandbox' else 'SPECTRE_API'
        decl = f"{macro} {ret} {name}({args});"
        
        # Add to defs
        defs.append(body.replace('static inline ', ''))
        
        h_content = h_content[:start] + decl + h_content[close_brace+1:]

    with open(h_file, 'w') as f: f.write(h_content)

    # 2. SDK API
    sdk_defs = []
    if os.path.exists(sdk_file):
        with open(sdk_file, 'r') as f: sdk_content = f.read()
        
        sdk_func_pattern = re.compile(r'^[ \t]*static[ \t]+(?!template)(.*?)\s+(\w+)\((.*?)\)\s*\{', re.MULTILINE)
        
        while True:
            m = sdk_func_pattern.search(sdk_content)
            if not m: break
            
            # Check for template above
            pre_text = sdk_content[:m.start()]
            if 'template' in pre_text.split('\n')[-2]:
                sdk_content = sdk_content[:m.start()] + sdk_content[m.start():].replace('static ', 'STATIC_SKIP ', 1)
                continue
                
            start = m.start()
            open_brace = m.end() - 1
            close_brace = extract_braces(sdk_content, open_brace)
            if close_brace == -1: break
            
            ret = m.group(1).strip()
            name = m.group(2).strip()
            args = m.group(3).strip()
            
            if '<' in name or '<' in ret:
                sdk_content = sdk_content[:m.start()] + sdk_content[m.start():].replace('static ', 'STATIC_SKIP ', 1)
                continue
            
            body = sdk_content[start:close_brace+1]
            
            decl = f"static {ret} {name}({args});"
            
            class_name = os.path.basename(sdk_file).replace('.hpp', '')
            
            # Remove default arguments from the definition arguments!
            clean_args = remove_default_args(args)
            
            def_body = body.strip()
            # Replace the signature in the body
            sig_pattern = f"static {ret} {name}({args})"
            # It's safer to reconstruct it:
            body_inner = body[body.find('{'):]
            def_body = f"{ret} {class_name}::{name}({clean_args}) {body_inner}"
            
            sdk_defs.append(def_body)
            
            sdk_content = sdk_content[:start] + '    ' + decl + sdk_content[close_brace+1:]
            
        sdk_content = sdk_content.replace('STATIC_SKIP ', 'static ')
        with open(sdk_file, 'w') as f: f.write(sdk_content)

    # Append to cpp
    if os.path.exists(cpp_file):
        with open(cpp_file, 'r') as f: cpp_content = f.read()
        cpp_content = re.sub(r'// --- SDK Implementations ---.*', '', cpp_content, flags=re.DOTALL)
        cpp_content = re.sub(r'// --- Public C API Implementations ---.*', '', cpp_content, flags=re.DOTALL)
        cpp_content = cpp_content.strip() + '\n'
        
        if sdk_file:
            inc_str = f'#include <{prefix}/sdk/{mod}.hpp>'
            if inc_str not in cpp_content:
                cpp_content = f'{inc_str}\n' + cpp_content
                
        if defs:
            cpp_content += '\n// --- Public C API Implementations ---\n' + '\n\n'.join(defs) + '\n'
            
        if sdk_defs:
            ns = f"{prefix}::modules"
            cpp_content += f'\n// --- SDK Implementations ---\nnamespace {ns} {{\n'
            cpp_content += '\n\n'.join(sdk_defs)
            cpp_content += '\n}\n'
            
        with open(cpp_file, 'w') as f: f.write(cpp_content)


def run():
    import subprocess
    subprocess.run("git checkout modules/core/include/sandbox/sdk modules/core/include/sandbox/services modules/core/source", shell=True, cwd='/home/jehud/CLionProjects/sandbox')
    subprocess.run("git checkout spectre/include/spectre/sdk spectre/include/spectre/services spectre/source/modules", shell=True, cwd='/home/jehud/CLionProjects/spectre')

    s_base = '/home/jehud/CLionProjects/sandbox/modules/core'
    for mod in ['application', 'runtime', 'configuration', 'logs', 'filesystem']:
        process_file(f'{s_base}/include/sandbox/services/{mod}_service.h',
                     f'{s_base}/source/{mod}/{mod}_service.cpp',
                     f'{s_base}/include/sandbox/sdk/{mod}.hpp', 'sandbox', mod)

    sp_base = '/home/jehud/CLionProjects/spectre/spectre'
    for mod in ['window', 'renderer', 'resources', 'serializer', 'scripts', 'prefabs', 'scenes']:
        process_file(f'{sp_base}/include/spectre/services/{mod}_service.h',
                     f'{sp_base}/source/modules/{mod}/{mod}_service.cpp',
                     f'{sp_base}/include/spectre/sdk/{mod}.hpp', 'spectre', mod)

run()
