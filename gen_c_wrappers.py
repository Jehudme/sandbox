import os
import re

def process_sdk_content(sdk_content, funcs, prefix):
    # For each func, find the matching SANDBOX_GET_SERVICE block in SDK and replace it
    for f in funcs:
        c_name = f"{prefix}_{f['name']}"
        ret = f['ret_type']
        ret_kw = "return " if ret != "void" else ""
        
        # We need to map SDK args to C wrapper args. The easiest way is to find `service->api->func_name(...)`
        # and replace the whole `if (service) { ... }` block with `return c_name(...)`.
        
        pattern = re.compile(rf'if\s*\(\s*const\s+auto\s*\*\s*service\s*=\s*SANDBOX_GET_SERVICE\s*\([^,]+,\s*\w+_service_t\)\s*\)\s*\{{\s*if\s*\(\s*service->api\s*&&\s*service->api->{f["name"]}\s*\)\s*\{{\s*({ret_kw})service->api->{f["name"]}\((.*?)\);\s*(?:return;)?\s*\}}\s*\}}\s*(?:return[^;]*;)?', re.DOTALL)
        
        def repl(match):
            has_return = match.group(1) if match.group(1) else ""
            args = match.group(2)
            
            call_stmt = f"{has_return}{c_name}({args});"
            if ret == "void" and not has_return:
                pass
            return call_stmt
            
        sdk_content = pattern.sub(repl, sdk_content)
        
        # There might be some alternative forms where it's `const xyz_service_t* service = SANDBOX_GET_SERVICE(...)`
        pattern2 = re.compile(rf'const\s+\w+_service_t\s*\*\s*service\s*=\s*SANDBOX_GET_SERVICE\s*\([^,]+,\s*\w+_service_t\);\s*if\s*\(\s*service\s*&&\s*service->api\s*&&\s*service->api->{f["name"]}\s*\)\s*\{{\s*({ret_kw})service->api->{f["name"]}\((.*?)\);\s*(?:return;)?\s*\}}\s*(?:return[^;]*;)?', re.DOTALL)
        sdk_content = pattern2.sub(repl, sdk_content)
        
    return sdk_content

def process_file_full(h_file, cpp_file, sdk_file, project_name):
    # [Read files]
    with open(h_file, 'r') as f: h_content = f.read()
    struct_pattern = re.compile(rf'typedef struct ({project_name}_\w+)_api_t')
    m = struct_pattern.search(h_content)
    if not m: return
    prefix = m.group(1)
    
    # [Parse API]
    struct_pattern_full = re.compile(rf'typedef struct {prefix}_api_t \{{(.*?)\}} {prefix}_api_t;', re.DOTALL)
    m = struct_pattern_full.search(h_content)
    if not m: return
    body = m.group(1)
    func_pattern = re.compile(r'^\s*(.*?)\s*\(\*(\w+)\)\((.*?)\);', re.MULTILINE)
    funcs = []
    for match in func_pattern.finditer(body):
        ret_type = match.group(1).strip()
        name = match.group(2).strip()
        args = match.group(3).strip()
        arg_names = []
        if args != 'void' and args != '':
            for arg in args.split(','):
                arg_names.append(arg.strip().split()[-1].lstrip('*&').split('[')[0])
        funcs.append({'ret_type': ret_type, 'name': name, 'args': args, 'arg_names': arg_names})

    # [Gen C wrappers]
    decls, impls = [], []
    for f in funcs:
        c_name = f"{prefix}_{f['name']}"
        decls.append(f"SANDBOX_API {f['ret_type']} {c_name}({f['args']});")
        args_call = ', '.join(f['arg_names'])
        ret_stmt = 'return ' if f['ret_type'] != 'void' else ''
        default_ret = ''
        if f['ret_type'] != 'void':
            if f['ret_type'] == 'bool': default_ret = 'return false;'
            elif '*' in f['ret_type']: default_ret = 'return nullptr;'
            elif any(x in f['ret_type'] for x in ['int', 'float', 'double', 'size_t', 'char']): default_ret = 'return 0;'
            else: default_ret = 'return {0};'
            
        world_arg = f['arg_names'][0] if f['arg_names'] else 'world'
        impl = f"""{f['ret_type']} {c_name}({f['args']}) {{
    if (const auto* service = SANDBOX_GET_SERVICE({world_arg}, {prefix}_service_t)) {{
        if (service->api && service->api->{f['name']}) {{
            {ret_stmt}service->api->{f['name']}({args_call});
            {"return;" if f['ret_type'] == 'void' else ""}
        }}
    }}
    {default_ret}
}}"""
        impls.append(impl)

    # [Patch Header]
    decls_str = '\n'.join(decls)
    old_c_pattern = re.compile(r'// --- Public C API ---\n.*?typedef struct', re.DOTALL)
    if old_c_pattern.search(h_content):
        h_content = old_c_pattern.sub(f'// --- Public C API ---\n{decls_str}\n\ntypedef struct', h_content)
    else:
        h_content = h_content.replace(f'typedef struct {prefix}_api_t', f'// --- Public C API ---\n{decls_str}\n\ntypedef struct {prefix}_api_t')
    with open(h_file, 'w') as f: f.write(h_content)

    # [Patch CPP]
    if cpp_file and os.path.exists(cpp_file):
        with open(cpp_file, 'r') as f: cpp_content = f.read()
        old_impl_pattern = re.compile(r'// --- Public C API Implementations ---.*', re.DOTALL)
        if old_impl_pattern.search(cpp_content):
            cpp_content = old_impl_pattern.sub('', cpp_content)
        impls_str = '\n'.join(impls)
        cpp_content = cpp_content.strip() + f"\n\n// --- Public C API Implementations ---\n{impls_str}\n"
        with open(cpp_file, 'w') as f: f.write(cpp_content)

    # [Patch SDK]
    if sdk_file and os.path.exists(sdk_file):
        with open(sdk_file, 'r') as f: sdk_content = f.read()
        sdk_content = process_sdk_content(sdk_content, funcs, prefix)
        with open(sdk_file, 'w') as f: f.write(sdk_content)

def run():
    # Sandbox
    s_base = '/home/jehud/CLionProjects/sandbox/modules/core'
    s_mods = ['application', 'runtime', 'configuration', 'logs', 'filesystem']
    for mod in s_mods:
        process_file_full(f'{s_base}/include/sandbox/services/{mod}_service.h',
                          f'{s_base}/source/{mod}/{mod}_service.cpp',
                          f'{s_base}/include/sandbox/sdk/{mod}.hpp', 'sandbox')

    # Spectre
    sp_base = '/home/jehud/CLionProjects/spectre/spectre'
    sp_mods = ['window', 'renderer', 'resources', 'serializer', 'scripts', 'prefabs', 'scenes']
    for mod in sp_mods:
        process_file_full(f'{sp_base}/include/spectre/services/{mod}_service.h',
                          f'{sp_base}/source/modules/{mod}/{mod}_service.cpp',
                          f'{sp_base}/include/spectre/sdk/{mod}.hpp', 'spectre')

run()
