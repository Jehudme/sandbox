#pragma once
#include <sandbox/abi/bootstrapper.h>
#include <sandbox/abi/properties.h>
#include <sandbox/abi/handle.h>

extern "C" {
    SANDBOX_DEFINE_HANDLE(sandbox_file_handle_t);

    typedef struct sandbox_filesystem_api_t {
        bool (*mount)(ecs_world_t* ecs, const char* physical, const char* virt, bool readonly);
    } sandbox_filesystem_api_t;

    SANDBOX_DECLARE_SERVICE(sandbox_filesystem_service_t, sandbox_filesystem_api_t, {
        .struct_size = 0,
        .name = "filesystem",
        .description = "Filesystem module service",
        .architecture = "sandbox::core",
        .version_major = 1,
        .version_minor = 0,
        .init_fn = NULL
    });
}
