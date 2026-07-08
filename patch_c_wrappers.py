import os
import re

def patch_c_wrappers(h_file, prefix):
    with open(h_file, 'r') as f:
        content = f.read()

    # Pattern to match: const sandbox_application_service_t* service = (const sandbox_application_service_t*)SANDBOX_GET_SERVICE(world, sandbox_application_service_t);
    # Or with ecs_singleton_get
    
    pattern = re.compile(rf'\s*const\s+{prefix}_service_t\*\s*service\s*=\s*\(const\s+{prefix}_service_t\*\)(?:SANDBOX_GET_SERVICE|ecs_singleton_get)\(([^,]+),\s*{prefix}_service_t\);')
    
    def repl(m):
        world_arg = m.group(1)
        return f"""
#ifdef __cplusplus
    flecs::world flecs_world({world_arg});
    const {prefix}_service_t* service = flecs_world.try_get<{prefix}_service_t>();
#else
    const {prefix}_service_t* service = (const {prefix}_service_t*)ecs_singleton_get({world_arg}, {prefix}_service_t);
#endif"""
        
    new_content = pattern.sub(repl, content)
    
    if new_content != content:
        with open(h_file, 'w') as f:
            f.write(new_content)
        print(f"Patched {h_file}")

def run():
    s_base = '/home/jehud/CLionProjects/sandbox/modules/core/include/sandbox/services'
    for mod in ['application', 'runtime', 'configuration', 'logs', 'filesystem']:
        patch_c_wrappers(f'{s_base}/{mod}_service.h', 'sandbox_' + mod)

    sp_base = '/home/jehud/CLionProjects/spectre/spectre/include/spectre/services'
    for mod in ['window', 'renderer', 'resources', 'serializer', 'scripts', 'prefabs', 'scenes']:
        patch_c_wrappers(f'{sp_base}/{mod}_service.h', 'spectre_' + mod)

run()
