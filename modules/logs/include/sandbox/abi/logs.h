#pragma once

#include <sandbox/abi/bootstrapper.h>

#ifdef __cplusplus
extern "C" {
#endif

// The API for the logs service
typedef struct {
    void (*trace)(ecs_world_t* ecs, const char* msg);
    void (*debug)(ecs_world_t* ecs, const char* msg);
    void (*info)(ecs_world_t* ecs, const char* msg);
    void (*warn)(ecs_world_t* ecs, const char* msg);
    void (*error)(ecs_world_t* ecs, const char* msg);
} sandbox_logs_api_t;

SANDBOX_DECLARE_SERVICE(sandbox_logs_service_t, sandbox_logs_api_t)

#ifdef __cplusplus
}
#endif
