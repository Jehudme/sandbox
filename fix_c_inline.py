import os
import re

def process_file_full(h_file, cpp_file, sdk_file, project_name):
    with open(h_file, 'r') as f: h_content = f.read()
    struct_pattern = re.compile(rf'typedef struct ({project_name}_\w+)_api_t')
    m = struct_pattern.search(h_content)
    if not m: return
    prefix = m.group(1)
    
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

    # [Gen pure C static inline wrappers]
    impls = []
    for f in funcs:
        c_name = f"{prefix}_{f['name']}"
        args_call = ', '.join(f['arg_names'])
        ret_stmt = 'return ' if f['ret_type'] != 'void' else ''
        default_ret = ''
        if f['ret_type'] != 'void':
            if f['ret_type'] == 'bool': default_ret = 'return false;'
            elif '*' in f['ret_type']: default_ret = 'return NULL;'
            elif any(x in f['ret_type'] for x in ['int', 'float', 'double', 'size_t', 'char']): default_ret = 'return 0;'
            else: default_ret = f"return ({f['ret_type']}){{0}};" # C struct literal cast
            # Handle sandbox_properties_handle_t safely
            if f['ret_type'] == 'sandbox_properties_handle_t':
                default_ret = 'sandbox_properties_handle_t invalid = {0}; return invalid;'
                
        world_arg = f['arg_names'][0] if f['arg_names'] else 'world'
        impl = f"""static inline {f['ret_type']} {c_name}({f['args']}) {{
    const {prefix}_service_t* service = (const {prefix}_service_t*)SANDBOX_GET_SERVICE({world_arg}, {prefix}_service_t);
    if (service && service->api && service->api->{f['name']}) {{
        {ret_stmt}service->api->{f['name']}({args_call});
        {"return;" if f['ret_type'] == 'void' else ""}
    }}
    {default_ret}
}}"""
        impls.append(impl)

    impls_str = '\n'.join(impls)
    
    # [Patch Header]
    # Remove the old decls block
    old_c_pattern = re.compile(r'// --- Public C API ---\n.*?\n\ntypedef struct', re.DOTALL)
    if old_c_pattern.search(h_content):
        h_content = old_c_pattern.sub(f'// --- Public C API ---\n{impls_str}\n\ntypedef struct', h_content)
    
    with open(h_file, 'w') as f: f.write(h_content)

    # [Patch CPP] - Remove the old implementations
    if cpp_file and os.path.exists(cpp_file):
        with open(cpp_file, 'r') as f: cpp_content = f.read()
        old_impl_pattern = re.compile(r'// --- Public C API Implementations ---.*', re.DOTALL)
        if old_impl_pattern.search(cpp_content):
            cpp_content = old_impl_pattern.sub('', cpp_content)
            with open(cpp_file, 'w') as f: f.write(cpp_content.strip() + '\n')

def run():
    s_base = '/home/jehud/CLionProjects/sandbox/modules/core'
    for mod in ['application', 'runtime', 'configuration', 'logs', 'filesystem']:
        process_file_full(f'{s_base}/include/sandbox/services/{mod}_service.h',
                          f'{s_base}/source/{mod}/{mod}_service.cpp', None, 'sandbox')

    sp_base = '/home/jehud/CLionProjects/spectre/spectre'
    for mod in ['window', 'renderer', 'resources', 'serializer', 'scripts', 'prefabs', 'scenes']:
        process_file_full(f'{sp_base}/include/spectre/services/{mod}_service.h',
                          f'{sp_base}/source/modules/{mod}/{mod}_service.cpp', None, 'spectre')

run()
