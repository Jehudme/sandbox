#pragma once

#include <sandbox/abi/bootstrapper.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*sandbox_event_callback_t)(const void* event, void* user_data);

typedef struct sandbox_events_api_t {
    void (*publish)(ecs_world_t* ecs, ecs_id_t event_id, const void* event);
    ecs_entity_t (*subscribe)(ecs_world_t* ecs, ecs_id_t event_id, sandbox_event_callback_t callback, void* user_data);
} sandbox_events_api_t;

SANDBOX_DECLARE_SERVICE(sandbox_events_service_t, sandbox_events_api_t, {
    .struct_size = 0,
    .name = "events",
    .description = "Global event pub/sub module",
    .architecture = "sandbox::core",
    .version_major = 1,
    .version_minor = 0
});

#ifdef __cplusplus
}
#endif
