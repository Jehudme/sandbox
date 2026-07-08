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
