#include <sandbox/sdk/engine.hpp>

#include <sandbox/abi/filesystem.h>
#include "filesystem.h"

extern "C" {
    static bool filesystem_mount(ecs_world_t* ecs, const char* physical, const char* virt, bool readonly) {
        flecs::world world(ecs);
        auto* fs = world.try_get_mut<sandbox::modules::filesystem>();
        if (fs) {
            try {
                return fs->mount(physical, virt, readonly);
            } catch(...) {
                return false;
            }
        }
        return false;
    }

    static sandbox_filesystem_api_t filesystem_api = {
        .mount = filesystem_mount
    };

    SANDBOX_DEFINE_SERVICE(sandbox_filesystem_service_t, sandbox_filesystem_api_t, &filesystem_api);
}

namespace sandbox::modules {
    SANDBOX_DECLARE_MODULE(filesystem, {
        .struct_size = 0,
        .name = "filesystem",
        .description = "Filesystem module",
        .architecture = "sandbox",
        .version_major = 1,
        .version_minor = 0,
        .version_patch = 0,
        .service = &sandbox_filesystem_service_t_info,
        .requirements = nullptr,
        .requirement_count = 0,
        .init_fn = nullptr
    });
}
