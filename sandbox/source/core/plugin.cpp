#include "sandbox/core/platform.h"
#include "sandbox/core/plugin.h"

#if !defined(_WIN32) && !defined(_WIN64) && !defined(__CYGWIN__)
    #include <dlfcn.h>
    #include <cstring>
    #include <cstdint>
    #include <iostream> // For printing the raw kernel error
#endif

namespace sandbox {

    void configure_plugin_os_api() {
        ecs_os_set_api_defaults();

#if defined(__linux__) || defined(__APPLE__)
        ecs_os_api_t os_api = ecs_os_api;

        // ====================================================================
        // FIX: Pass the exact file name provided without forcing an extension.
        // This prevents double-extensions (.so.so) and allows versioned
        // libraries (like .so.1) to load flawlessly via the OS linker.
        // ====================================================================
        os_api.module_to_dl_ = [](const char* module) -> char* {
            auto* result = static_cast<char*>(ecs_os_malloc(std::strlen(module) + 1));
            std::strcpy(result, module);
            return result;
        };

        // ====================================================================
        // CRITICAL UPDATE: Unmasking the Linux Linker!
        // Switched to RTLD_LAZY and added direct dlerror() printing.
        // ====================================================================
        os_api.dlopen_ = [](const char* lib) -> uintptr_t {
            void* handle = dlopen(lib, RTLD_LAZY | RTLD_GLOBAL);
            if (!handle) {
                const char* err = dlerror();
                std::cerr << "\n[OS Linker] FATAL REJECTION: dlopen failed for '" << lib << "'\n"
                          << "[OS Linker] EXACT REASON: " << (err ? err : "Unknown") << "\n\n";
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