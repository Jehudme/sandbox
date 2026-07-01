#pragma once
#include <sandbox/abi/bootstrapper.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sandbox_application_api_t {
    bool (*is_running)(ecs_world_t* ecs);
} sandbox_application_api_t;

SANDBOX_DECLARE_SERVICE(sandbox_application_service_t, sandbox_application_api_t, {
    .struct_size = 0,
    .name = "application",
    .description = "Application lifecycle service",
    .architecture = "sandbox",
    .version_major = 1,
    .version_minor = 0,
    .init_fn = NULL
});

#ifdef __cplusplus
}
#endif
