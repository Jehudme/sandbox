with open('/home/jehud/CLionProjects/sandbox/modules/core/include/sandbox/sdk/configuration.hpp', 'r') as f:
    content = f.read()

import re

# We want to replace get_properties body.
pattern = r'(static sandbox::properties get_properties\(flecs::world& entity_world\) \{)[\s\S]*?(sandbox_properties_handle_t invalid = \{0\};\s*return sandbox::properties\(invalid, false\);\s*\})'

def repl(m):
    return m.group(1) + """
            return sandbox::properties(sandbox_configuration_get_properties(entity_world.c_ptr()), false);
        }"""

new_content = re.sub(pattern, repl, content)
with open('/home/jehud/CLionProjects/sandbox/modules/core/include/sandbox/sdk/configuration.hpp', 'w') as f:
    f.write(new_content)
