#include <sandbox/sdk/application.hpp>
#include <sandbox/sdk/logs.hpp>
#include "sandbox/services/application_service.h"
#include "application_module.h"
#include <flecs.h>

// ABI methods
    static bool application_is_running(ecs_world_t* entity_world) {
        (void)entity_world;
        return true; 
    }

    static sandbox_application_api_t application_api = {
        .is_running = application_is_running
    };

    SANDBOX_DEFINE_SERVICE(sandbox_application_service_t, sandbox_application_api_t, &application_api);

// --- Public C API Implementations ---
bool sandbox_application_is_running(ecs_world_t* ecs) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_application_service_t* service = flecs_world.try_get<sandbox_application_service_t>();
#else
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_application_service_t* service = flecs_world.try_get<sandbox_application_service_t>();
#else
    const sandbox_application_service_t* service = (const sandbox_application_service_t*)ecs_singleton_get(ecs, sandbox_application_service_t);
#endif
#endif
    if (service && service->api && service->api->is_running) {
        return service->api->is_running(ecs);
        
    } else {
        sandbox::modules::logs::error(flecs_world, "[Application Module] Service not initialized!");
    }
    return false;
}

// --- SDK Implementations ---
namespace sandbox::modules {
bool application::is_running(const flecs::world& entity_world) {
            return sandbox_application_is_running(entity_world.c_ptr());
        }
}
