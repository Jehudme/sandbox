#include "sandbox/services/runtime_service.h"
#include "runtime_module.h"
#include <flecs.h>

// C-ABI Endpoints
// ==========================================

static void runtime_run(ecs_world_t* entity_world);
static void runtime_start(ecs_world_t* entity_world);
static void runtime_stop(ecs_world_t* entity_world);
static void runtime_pause(ecs_world_t* entity_world);
static void runtime_resume(ecs_world_t* entity_world);

sandbox_runtime_api_t g_runtime_api = {
    .run = runtime_run,
    .start = runtime_start,
    .stop = runtime_stop,
    .pause = runtime_pause,
    .resume = runtime_resume
};

SANDBOX_DEFINE_SERVICE(sandbox_runtime_service_t, sandbox_runtime_api_t, &g_runtime_api)

static void runtime_run(ecs_world_t* entity_world) {
    if (!entity_world) return;
    flecs::world flecs_world(entity_world);
    auto* runtime = flecs_world.try_get_mut<sandbox::modules::runtime_t>();
    if (runtime) runtime->run(flecs_world);
}

static void runtime_start(ecs_world_t* entity_world) {
    if (!entity_world) return;
    flecs::world flecs_world(entity_world);
    auto* runtime = flecs_world.try_get_mut<sandbox::modules::runtime_t>();
    if (runtime) runtime->start(flecs_world);
}

static void runtime_stop(ecs_world_t* entity_world) {
    if (!entity_world) return;
    flecs::world flecs_world(entity_world);
    auto* runtime = flecs_world.try_get_mut<sandbox::modules::runtime_t>();
    if (runtime) runtime->stop();
}

static void runtime_pause(ecs_world_t* entity_world) {
    if (!entity_world) return;
    flecs::world flecs_world(entity_world);
    auto* runtime = flecs_world.try_get_mut<sandbox::modules::runtime_t>();
    if (runtime) runtime->pause();
}

static void runtime_resume(ecs_world_t* entity_world) {
    if (!entity_world) return;
    flecs::world flecs_world(entity_world);
    auto* runtime = flecs_world.try_get_mut<sandbox::modules::runtime_t>();
    if (runtime) runtime->resume();
}
