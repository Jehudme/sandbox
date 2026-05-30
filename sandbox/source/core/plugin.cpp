#include "sandbox/core/plugin.h"

#if !defined(_WIN32) && !defined(_WIN64) && !defined(__CYGWIN__)
    #include <dlfcn.h>
    #include <cstring>
    #include <cstdint>
#endif

namespace sandbox {

    void configure_plugin_os_api() {
        ecs_os_set_api_defaults();

#if defined(__linux__) || defined(__APPLE__)
        ecs_os_api_t os_api = ecs_os_api;

        os_api.module_to_dl_ = [](const char* module) -> char* {
            std::string name = std::string(module) + SANDBOX_COMPATIBLE_MODULE_EXTENSION;
            auto* result = static_cast<char*>(ecs_os_malloc(name.length() + 1));
            std::strcpy(result, name.c_str());
            return result;
        };

        os_api.dlopen_ = [](const char* lib) -> uintptr_t {
            return reinterpret_cast<uintptr_t>(dlopen(lib, RTLD_NOW | RTLD_GLOBAL));
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