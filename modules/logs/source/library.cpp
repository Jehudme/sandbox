#include <sandbox/abi/logs.h>
#include <sandbox/sdk/logs.hpp>
#include "logger.h"
#include <flecs/addons/cpp/flecs.hpp>
#include <spdlog/spdlog.h>

// Forward declaration
namespace sandbox::modules {
    struct logs_module_t;
}
typedef sandbox::modules::logs_module_t sandbox_logs_module_t;

// API Implementations
static void logs_trace(ecs_world_t* ecs, const char* msg);
static void logs_debug(ecs_world_t* ecs, const char* msg);
static void logs_info(ecs_world_t* ecs, const char* msg);
static void logs_warn(ecs_world_t* ecs, const char* msg);
static void logs_error(ecs_world_t* ecs, const char* msg);

sandbox_logs_api_t g_logs_api = {
    .trace = logs_trace,
    .debug = logs_debug,
    .info = logs_info,
    .warn = logs_warn,
    .error = logs_error
};

SANDBOX_DEFINE_SERVICE(sandbox_logs_service_t, sandbox_logs_api_t, &g_logs_api)

static void logs_trace(ecs_world_t* ecs, const char* msg) {
    flecs::world world(ecs);
    auto* log = world.try_get_mut<sandbox::modules::logger>();
    if (log) log->log(sandbox::modules::logger::level::TRACE, msg);
}
static void logs_debug(ecs_world_t* ecs, const char* msg) {
    flecs::world world(ecs);
    auto* log = world.try_get_mut<sandbox::modules::logger>();
    if (log) log->log(sandbox::modules::logger::level::DEBUG, msg);
}
static void logs_info(ecs_world_t* ecs, const char* msg) {
    flecs::world world(ecs);
    auto* log = world.try_get_mut<sandbox::modules::logger>();
    if (log) log->log(sandbox::modules::logger::level::INFO, msg);
}
static void logs_warn(ecs_world_t* ecs, const char* msg) {
    flecs::world world(ecs);
    auto* log = world.try_get_mut<sandbox::modules::logger>();
    if (log) log->log(sandbox::modules::logger::level::WARN, msg);
}
static void logs_error(ecs_world_t* ecs, const char* msg) {
    flecs::world world(ecs);
    auto* log = world.try_get_mut<sandbox::modules::logger>();
    if (log) log->log(sandbox::modules::logger::level::ERROR, msg);
}

static sandbox_requirement_info_t logs_requirements[] = {
    {
        .kind = SANDBOX_REQUIREMENT_KIND_SERVICE,
        .strictness = SANDBOX_REQUIREMENT_STRICTNESS_REQUIRED,
        .name = "configuration",
        .architecture = "sandbox",
        .version_major = 1,
        .version_minor = 0,
        .version_patch = -1
    }
};


namespace sandbox::modules {
    SANDBOX_DECLARE_MODULE(logger, {
    .name = "logs",
    .description = "Global logging module",
    .architecture = "sandbox",
    .version_major = 1,
    .version_minor = 0,
    .version_patch = 0,
    .service = &sandbox_logs_service_t_info,
    .requirements = logs_requirements,
    .requirement_count = 1
})
}
