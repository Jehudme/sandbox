#!/bin/bash
set -ex
cd /home/jehud/CLionProjects/sandbox/sandbox

# Rename schemas
mv src/schemas schemas

# Rename src to source
mv src source

# Move detail inlines to include
mkdir -p include/sandbox/event_bus/detail
mv source/event_bus/detail/event_bus.inl include/sandbox/event_bus/detail/event_bus.inl
rm -rf source/event_bus

mkdir -p include/sandbox/utilities/detail
mv source/utilities/events.inl include/sandbox/utilities/events.inl
mv source/utilities/detail/properties.inl include/sandbox/utilities/detail/properties.inl
rm -rf source/utilities/detail

# Move public macros and wrappers to include/sandbox/core
mkdir -p include/sandbox/core
mv include/sandbox/api/payload.h include/sandbox/core/payload.h
mv include/sandbox/api/sdk_wrappers.h include/sandbox/core/sdk_wrappers.h
mv include/sandbox/generated/schemas/common_generated.h include/sandbox/core/common_schema.h

# Move API interfaces to modules/
mkdir -p include/sandbox/modules/logger
mv include/sandbox/api/logger_api.h include/sandbox/modules/logger/logger_api.h
mv source/subsystems/logger/ilogger.h include/sandbox/modules/logger/ilogger.h
mv include/sandbox/generated/schemas/logger_generated.h include/sandbox/modules/logger/logger_schema.h

mkdir -p include/sandbox/modules/filesystem
mv include/sandbox/api/filesystem_api.h include/sandbox/modules/filesystem/filesystem_api.h
mv source/subsystems/filesystem/ifilesystem.h include/sandbox/modules/filesystem/ifilesystem.h
mv include/sandbox/generated/schemas/filesystem_generated.h include/sandbox/modules/filesystem/filesystem_schema.h

mkdir -p include/sandbox/modules/runner
mv include/sandbox/api/runner_api.h include/sandbox/modules/runner/runner_api.h
mv source/subsystems/runner/irunner.h include/sandbox/modules/runner/irunner.h
mv include/sandbox/generated/schemas/runner_generated.h include/sandbox/modules/runner/runner_schema.h

# Cleanup old include dirs
rm -rf include/sandbox/api/abi_types.h
rm -rf include/sandbox/api
rm -rf include/sandbox/generated

# Move subsystems to modules in source
mv source/subsystems source/modules

# Write the new macro file
cat << 'EOF' > include/sandbox/core/service_macro.h
#pragma once
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DECLARE_SANDBOX_SERVICE(ServiceName) \
struct ServiceName { \
    void* instance; \
    void (*execute_command)(void* instance, uint32_t command_id, const uint8_t* payload, size_t size); \
};

#ifdef __cplusplus
}
#endif
EOF
