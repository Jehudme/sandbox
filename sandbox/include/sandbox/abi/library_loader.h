#pragma once

#include <sandbox/abi/bootstrapper.h>

#ifdef __cplusplus
extern "C" {
#endif

    /**
     * @brief API for the library loader service.
     */
    typedef struct sandbox_library_loader_api_t {
        void (*load)(ecs_world_t* ecs, const char* path);
        void (*load_from_memory)(ecs_world_t* ecs, const uint8_t* library_data, size_t size);
        void (*unload)(ecs_world_t* ecs, const char* library_name);
    } sandbox_library_loader_api_t;

    /**
     * @brief The library loader service definition.
     */
    SANDBOX_DECLARE_SERVICE(sandbox_library_loader_service_t, sandbox_library_loader_api_t, {
        .struct_size = 0,
        .name = "library_loader",
        .description = "Dynamic library loader service",
        .architecture = "sandbox::core",
        .version_major = 1,
        .version_minor = 0,
        .init_fn = NULL
    });

#ifdef __cplusplus
}
#endif
