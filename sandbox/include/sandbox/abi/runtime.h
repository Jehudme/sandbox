#pragma once

#include <sandbox/abi/platform.h>
#include <sandbox/abi/bootstrapper.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sandbox_runtime_api_t {
    void (*run)(ecs_world_t* ecs);
    void (*start)(ecs_world_t* ecs);
    void (*stop)(ecs_world_t* ecs);
    void (*pause)(ecs_world_t* ecs);
    void (*resume)(ecs_world_t* ecs);
} sandbox_runtime_api_t;

SANDBOX_DECLARE_SERVICE(sandbox_runtime_service_t, sandbox_runtime_api_t, {
    .name = "runtime",
    .description = "Global runtime module service",
    .architecture = "sandbox",
    .version_major = 1,
    .version_minor = 0,
})

#ifdef __cplusplus
}
#endif
