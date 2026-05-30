#pragma once

#include "platform.h"
#include <flecs.h>

namespace sandbox::detail {

    template<typename T>
    inline void bootstrap_module(ecs_world_t* raw_world) {
        flecs::world{raw_world}.import<T>();
    }

    template<typename DummyType, typename RealModule>
    inline void bootstrap_library(ecs_world_t* raw_world, const char* expected_name) {
        flecs::world world{raw_world};

        world.import<RealModule>();
        world.module<DummyType>(expected_name);
    }

} // namespace sandbox::detail


#define SANDBOX_DEFINE_MODULE(ModuleClass, ModuleName)                         \
extern "C" SANDBOX_EXPORT void ModuleName##Import(ecs_world_t* world) {        \
    sandbox::detail::bootstrap_module<ModuleClass>(world);                     \
}                                                                              \
extern "C" SANDBOX_EXPORT void ModuleName(ecs_world_t* world) {                \
    sandbox::detail::bootstrap_module<ModuleClass>(world);                     \
}

#define SANDBOX_DEFINE_LIBRARY(MasterModuleClass)                              \
struct SandboxLibraryMain_Dummy {};                                            \
extern "C" SANDBOX_EXPORT void SandboxLibraryMainImport(ecs_world_t* world) {  \
    sandbox::detail::bootstrap_library<SandboxLibraryMain_Dummy, MasterModuleClass>(world, "SandboxLibraryMain"); \
}                                                                              \
extern "C" SANDBOX_EXPORT void SandboxLibraryMain(ecs_world_t* world) {        \
    sandbox::detail::bootstrap_library<SandboxLibraryMain_Dummy, MasterModuleClass>(world, "SandboxLibraryMain"); \
}

namespace sandbox {
    void configure_plugin_os_api();
}