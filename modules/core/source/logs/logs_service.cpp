#include "sandbox/services/logs_service.h"
#include "logs_module.h"
#include <spdlog/spdlog.h>
#include <flecs.h>

// C-ABI Endpoints
// ==========================================

// Forward declaration
namespace sandbox::modules {
    struct logs_module_t;
}
typedef sandbox::modules::logs_module_t sandbox_logs_module_t;

// API Implementations
static void logs_trace(ecs_world_t* entity_world, const char* msg);
static void logs_debug(ecs_world_t* entity_world, const char* msg);
static void logs_info(ecs_world_t* entity_world, const char* msg);
static void logs_warn(ecs_world_t* entity_world, const char* msg);
static void logs_error(ecs_world_t* entity_world, const char* msg);

sandbox_logs_api_t g_logs_api = {
    .trace = logs_trace,
    .debug = logs_debug,
    .info = logs_info,
    .warn = logs_warn,
    .error = logs_error
};

SANDBOX_DEFINE_SERVICE(sandbox_logs_service_t, sandbox_logs_api_t, &g_logs_api)

static void logs_trace(ecs_world_t* entity_world, const char* msg) {
    flecs::world flecs_world(entity_world);
    auto* log = flecs_world.try_get_mut<sandbox::modules::logger_t>();
    if (log) log->log(sandbox::modules::logger_t::level_t::TRACE, msg);
}
static void logs_debug(ecs_world_t* entity_world, const char* msg) {
    flecs::world flecs_world(entity_world);
    auto* log = flecs_world.try_get_mut<sandbox::modules::logger_t>();
    if (log) log->log(sandbox::modules::logger_t::level_t::DEBUG, msg);
}
static void logs_info(ecs_world_t* entity_world, const char* msg) {
    flecs::world flecs_world(entity_world);
    auto* log = flecs_world.try_get_mut<sandbox::modules::logger_t>();
    if (log) log->log(sandbox::modules::logger_t::level_t::INFO, msg);
}
static void logs_warn(ecs_world_t* entity_world, const char* msg) {
    flecs::world flecs_world(entity_world);
    auto* log = flecs_world.try_get_mut<sandbox::modules::logger_t>();
    if (log) log->log(sandbox::modules::logger_t::level_t::WARN, msg);
}
static void logs_error(ecs_world_t* entity_world, const char* msg) {
    flecs::world flecs_world(entity_world);
    auto* log = flecs_world.try_get_mut<sandbox::modules::logger_t>();
    if (log) log->log(sandbox::modules::logger_t::level_t::ERROR, msg);
}
