#pragma once
#include <flecs/addons/cpp/world.hpp>
#include <sandbox/abi/bootstrapper.h>

namespace sandbox::launcher {

    // ABI Service Definition
    extern "C" {
        typedef struct sandbox_application_api_t {
            bool (*is_running)(ecs_world_t* ecs);
        } sandbox_application_api_t;

        SANDBOX_DECLARE_SERVICE(sandbox_application_service_t, sandbox_application_api_t, {
            .struct_size = 0,
            .name = "application",
            .description = "Application lifecycle service",
            .architecture = "sandbox",
            .version_major = 1,
            .version_minor = 0,
            .init_fn = NULL
        });
    }

    struct application_t {
        application_t(flecs::world& ecs);
    };

}
