#pragma once
#include <sandbox/abi/bootstrapper.h>
#include <sandbox/abi/properties.h>
#include <sandbox/abi/handle.h>

extern "C" {
    /**
     * @brief An opaque handle representing an open file.
     */
    SANDBOX_DEFINE_HANDLE(sandbox_file_handle_t);

    /**
     * @brief API for the filesystem service.
     */
    typedef struct sandbox_filesystem_api_t {
        /**
         * @brief Mounts a physical path to a virtual mount point.
         * @param ecs The entity component system world.
         * @param physical The physical file path.
         * @param virt The virtual mount point.
         * @param readonly True if mounted read-only, false otherwise.
         * @return True if successful, false otherwise.
         */
        bool (*mount)(ecs_world_t* ecs, const char* physical, const char* virt, bool readonly);
    } sandbox_filesystem_api_t;

    /**
     * @brief The filesystem service definition.
     */
    SANDBOX_DECLARE_SERVICE(sandbox_filesystem_service_t, sandbox_filesystem_api_t, {
        .struct_size = 0,
        .name = "filesystem",
        .description = "Filesystem module service",
        .architecture = "sandbox::core",
        .version_major = 1,
        .version_minor = 0,
        .init_fn = NULL
    });
}
