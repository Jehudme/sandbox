#include <sandbox/abi/runtime.h>
#include <sandbox/sdk/runtime.hpp>
#include "runtime.h"
#include <flecs.h>

namespace sandbox::modules {
    struct runtime_module_t;
}
typedef sandbox::modules::runtime_module_t sandbox_runtime_module_t;

static void runtime_run(ecs_world_t* ecs);
static void runtime_start(ecs_world_t* ecs);
static void runtime_stop(ecs_world_t* ecs);
static void runtime_pause(ecs_world_t* ecs);
static void runtime_resume(ecs_world_t* ecs);

sandbox_runtime_api_t g_runtime_api = {
    .run = runtime_run,
    .start = runtime_start,
    .stop = runtime_stop,
    .pause = runtime_pause,
    .resume = runtime_resume
};

SANDBOX_DEFINE_SERVICE(sandbox_runtime_service_t, sandbox_runtime_api_t, &g_runtime_api)

static void runtime_run(ecs_world_t* ecs) {
    if (!ecs) return;
    flecs::world world(ecs);
    auto* runtime = world.try_get_mut<sandbox::modules::runtime_t>();
    if (runtime) runtime->run(world);
}

static void runtime_start(ecs_world_t* ecs) {
    if (!ecs) return;
    flecs::world world(ecs);
    auto* runtime = world.try_get_mut<sandbox::modules::runtime_t>();
    if (runtime) runtime->start(world);
}

static void runtime_stop(ecs_world_t* ecs) {
    if (!ecs) return;
    flecs::world world(ecs);
    auto* runtime = world.try_get_mut<sandbox::modules::runtime_t>();
    if (runtime) runtime->stop();
}

static void runtime_pause(ecs_world_t* ecs) {
    if (!ecs) return;
    flecs::world world(ecs);
    auto* runtime = world.try_get_mut<sandbox::modules::runtime_t>();
    if (runtime) runtime->pause();
}

static void runtime_resume(ecs_world_t* ecs) {
    if (!ecs) return;
    flecs::world world(ecs);
    auto* runtime = world.try_get_mut<sandbox::modules::runtime_t>();
    if (runtime) runtime->resume();
}

static sandbox_requirement_info_t runtime_requirements[] = {
    {
        .kind = SANDBOX_REQUIREMENT_KIND_SERVICE,
        .strictness = SANDBOX_REQUIREMENT_STRICTNESS_REQUIRED,
        .name = "logs",
        .architecture = "sandbox",
        .version_major = 1,
        .version_minor = 0,
        .version_patch = -1
    }
};

namespace sandbox::modules {
    SANDBOX_DECLARE_MODULE(runtime_module_t, {
        .name = "runtime",
        .description = "Global runtime module",
        .architecture = "sandbox",
        .version_major = 1,
        .version_minor = 0,
        .version_patch = 0,
        .service = &sandbox_runtime_service_t_info,
        .requirements = runtime_requirements,
        .requirement_count = 1
    })

    struct runtime_module_t {
        runtime_module_t(flecs::world& world) {
            world.component<runtime_t>();
            world.set<runtime_t>(runtime_t());
        }
    };
}
