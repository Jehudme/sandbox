#include <catch2/catch_session.hpp>
#include "sandbox/core/plugin.h"

int main(int argc, char* argv[]) {
    // CRITICAL: Configure Flecs OS API before any test cases run.
    // Tests will be instantiating fresh flecs::world objects and potentially
    // calling ecs_import_from_library for plugin loading tests. 
    // This ensures the custom dlopen/dlsym wrappers are registered first.
    sandbox::configure_plugin_os_api();

    Catch::Session session;
    int returnCode = session.applyCommandLine(argc, argv);
    if (returnCode != 0) {
        return returnCode;
    }

    return session.run();
}
