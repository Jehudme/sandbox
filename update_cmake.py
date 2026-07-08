import os
import re

cmake_path = '/home/jehud/CLionProjects/sandbox/modules/core/CMakeLists.txt'

with open(cmake_path, 'r') as f:
    content = f.read()

content = content.replace('source/configuration/configuration.cpp', 'source/configuration/configuration_module.cpp\n    source/configuration/configuration_service.cpp')
content = content.replace('source/filesystem/filesystem.cpp', 'source/filesystem/filesystem_module.cpp\n    source/filesystem/filesystem_service.cpp')
content = content.replace('source/logs/logger.cpp', 'source/logs/logs_module.cpp\n    source/logs/logs_service.cpp')
content = content.replace('source/runtime/runtime.cpp', 'source/runtime/runtime_module.cpp\n    source/runtime/runtime_service.cpp')
content = content.replace('source/application/application.cpp', 'source/application/application_module.cpp\n    source/application/application_service.cpp')

content = content.replace('source/configuration/configuration.h', 'source/configuration/configuration_module.h')
content = content.replace('source/filesystem/filesystem.h', 'source/filesystem/filesystem_module.h')
content = content.replace('source/logs/logger.h', 'source/logs/logs_module.h')
content = content.replace('source/runtime/runtime.h', 'source/runtime/runtime_module.h')
content = content.replace('source/application/application.h', 'source/application/application_module.h')

content = content.replace('include/sandbox/abi/configuration.h', 'include/sandbox/services/configuration_service.h')
content = content.replace('include/sandbox/abi/filesystem.h', 'include/sandbox/services/filesystem_service.h')
content = content.replace('include/sandbox/abi/logs.h', 'include/sandbox/services/logs_service.h')
content = content.replace('include/sandbox/abi/runtime.h', 'include/sandbox/services/runtime_service.h')
content = content.replace('include/sandbox/abi/application.h', 'include/sandbox/services/application_service.h')

content = re.sub(r'\s*include/sandbox/sdk/[a-zA-Z0-9_]+\.hpp', '', content)

with open(cmake_path, 'w') as f:
    f.write(content)

print("Updated CMakeLists.txt")
