#include <sandbox/sdk/logs.hpp>
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
void sandbox_logs_trace(ecs_world_t* entity_world, const char* msg) {
    if (!entity_world) return;
    flecs::world flecs_world(entity_world);
    auto* log = flecs_world.try_get_mut<sandbox::modules::logger_t>();
    if (log) log->log(sandbox::modules::logger_t::level_t::TRACE, msg);
}
void sandbox_logs_debug(ecs_world_t* entity_world, const char* msg) {
    if (!entity_world) return;
    flecs::world flecs_world(entity_world);
    auto* log = flecs_world.try_get_mut<sandbox::modules::logger_t>();
    if (log) log->log(sandbox::modules::logger_t::level_t::DEBUG, msg);
}
void sandbox_logs_info(ecs_world_t* entity_world, const char* msg) {
    if (!entity_world) return;
    flecs::world flecs_world(entity_world);
    auto* log = flecs_world.try_get_mut<sandbox::modules::logger_t>();
    if (log) log->log(sandbox::modules::logger_t::level_t::INFO, msg);
}
void sandbox_logs_warn(ecs_world_t* entity_world, const char* msg) {
    if (!entity_world) return;
    flecs::world flecs_world(entity_world);
    auto* log = flecs_world.try_get_mut<sandbox::modules::logger_t>();
    if (log) log->log(sandbox::modules::logger_t::level_t::WARN, msg);
}
void sandbox_logs_error(ecs_world_t* entity_world, const char* msg) {
    if (!entity_world) return;
    flecs::world flecs_world(entity_world);
    auto* log = flecs_world.try_get_mut<sandbox::modules::logger_t>();
    if (log) log->log(sandbox::modules::logger_t::level_t::ERROR, msg);
}

sandbox_logs_api_t g_logs_api = {
    .trace = sandbox_logs_trace,
    .debug = sandbox_logs_debug,
    .info = sandbox_logs_info,
    .warn = sandbox_logs_warn,
    .error = sandbox_logs_error
};

SANDBOX_DEFINE_SERVICE(sandbox_logs_service_t, sandbox_logs_api_t, &g_logs_api)

