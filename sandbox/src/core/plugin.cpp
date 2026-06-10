#include "sandbox/core/platform.h"
#include "sandbox/core/plugin.h"

#if !defined(_WIN32) && !defined(_WIN64) && !defined(__CYGWIN__)
    #include <dlfcn.h>
    #include <cstring>
    #include <cstdint>
#endif

#include <spdlog/spdlog.h>

namespace sandbox {

    /// Configures the ECS OS API with platform-specific dynamic linking functionality
    /// to support loading and executing external plugins at runtime.
    void configure_plugin_os_api() {
        ecs_os_set_api_defaults();

#if defined(__linux__) || defined(__APPLE__)
        ecs_os_api_t os_api = ecs_os_api;

        os_api.module_to_dl_ = [](const char* module) -> char* {
            auto* result = static_cast<char*>(ecs_os_malloc(std::strlen(module) + 1));
            std::strcpy(result, module);
            return result;
        };

        os_api.dlopen_ = [](const char* lib) -> uintptr_t {
            void* handle = dlopen(lib, RTLD_NOW | RTLD_GLOBAL);
            if (!handle) {
                const char* err = dlerror();
                const std::string msg = std::string("[Loader] dlopen failed for '") + lib
                                      + "': " + (err ? err : "Unknown error");
                // Route through the named spdlog logger if already registered,
                // fall back to stderr for early-boot failures.
                if (auto logger = spdlog::get("sandbox_core"); logger) {
                    logger->error(msg);
                } else {
                    spdlog::error(msg);
                }
            }
            return reinterpret_cast<uintptr_t>(handle);
        };

        os_api.dlproc_ = [](uintptr_t lib, const char* proc) -> void(*)() {
            return reinterpret_cast<void(*)()>(dlsym(reinterpret_cast<void*>(lib), proc));
        };

        os_api.dlclose_ = [](uintptr_t lib) {
            dlclose(reinterpret_cast<void*>(lib));
        };

        ecs_os_set_api(&os_api);
#endif
    }

} // namespace sandbox