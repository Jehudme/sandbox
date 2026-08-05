#include <sandbox/sdk/runtime.hpp>
#include <sandbox/sdk/logs.hpp>
#include "sandbox/services/runtime_service.h"
#include "runtime_module.h"
#include <flecs.h>
#include <iostream>

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
    if (runtime) runtime->stop(flecs_world);
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

// --- Public C API Implementations ---
void sandbox_runtime_run(ecs_world_t* ecs) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_runtime_service_t* service = (const sandbox_runtime_service_t*)ecs_get_id(ecs, flecs_world.id<sandbox_runtime_service_t>(), flecs_world.id<sandbox_runtime_service_t>());
#else
    const sandbox_runtime_service_t* service = (const sandbox_runtime_service_t*)ecs_singleton_get(ecs, sandbox_runtime_service_t);
#endif

    if (service && service->api && service->api->run) {
        service->api->run(ecs);
        return;
    } else {
#ifdef __cplusplus
        sandbox::modules::logs::error(flecs_world, "[Runtime Module] Service not initialized!");
#endif
    }
}

void sandbox_runtime_start(ecs_world_t* ecs) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_runtime_service_t* service = (const sandbox_runtime_service_t*)ecs_get_id(ecs, flecs_world.id<sandbox_runtime_service_t>(), flecs_world.id<sandbox_runtime_service_t>());
#else
    const sandbox_runtime_service_t* service = (const sandbox_runtime_service_t*)ecs_singleton_get(ecs, sandbox_runtime_service_t);
#endif

    if (service && service->api && service->api->start) {
        service->api->start(ecs);
    }
}

void sandbox_runtime_stop(ecs_world_t* ecs) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_runtime_service_t* service = (const sandbox_runtime_service_t*)ecs_get_id(ecs, flecs_world.id<sandbox_runtime_service_t>(), flecs_world.id<sandbox_runtime_service_t>());
#else
    const sandbox_runtime_service_t* service = (const sandbox_runtime_service_t*)ecs_singleton_get(ecs, sandbox_runtime_service_t);
#endif

    if (service && service->api && service->api->stop) {
        service->api->stop(ecs);
    }
}

void sandbox_runtime_pause(ecs_world_t* ecs) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_runtime_service_t* service = (const sandbox_runtime_service_t*)ecs_get_id(ecs, flecs_world.id<sandbox_runtime_service_t>(), flecs_world.id<sandbox_runtime_service_t>());
#else
    const sandbox_runtime_service_t* service = (const sandbox_runtime_service_t*)ecs_singleton_get(ecs, sandbox_runtime_service_t);
#endif

    if (service && service->api && service->api->pause) {
        service->api->pause(ecs);
    }
}

void sandbox_runtime_resume(ecs_world_t* ecs) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_runtime_service_t* service = (const sandbox_runtime_service_t*)ecs_get_id(ecs, flecs_world.id<sandbox_runtime_service_t>(), flecs_world.id<sandbox_runtime_service_t>());
#else
    const sandbox_runtime_service_t* service = (const sandbox_runtime_service_t*)ecs_singleton_get(ecs, sandbox_runtime_service_t);
#endif
    if (service && service->api && service->api->resume) {
        service->api->resume(ecs);
        return;
    } else {
        sandbox::modules::logs::error(flecs_world, "[Runtime Module] Service not initialized!");
    }
    
}

// --- SDK Implementations ---
namespace sandbox::modules {
void runtime::run(flecs::world& entity_world) {
            sandbox_runtime_run(entity_world.c_ptr());}

void runtime::start(flecs::world& entity_world) {
            sandbox_runtime_start(entity_world.c_ptr());}

void runtime::stop(flecs::world& entity_world) {
            sandbox_runtime_stop(entity_world.c_ptr());}

void runtime::pause(flecs::world& entity_world) {
            sandbox_runtime_pause(entity_world.c_ptr());}

void runtime::resume(flecs::world& entity_world) {
            sandbox_runtime_resume(entity_world.c_ptr());}
}
