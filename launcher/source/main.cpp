#include <iostream>
#include <filesystem>
#include <exception>

#include "sandbox/core/engine.h"
#include "sandbox/core/plugin.h"
#include "sandbox/macros/runner.h"

int main() {
    // 1. Configure OS-specific dynamic linking API
    sandbox::configure_plugin_os_api();

    // 2. Instantiate the engine core
    sandbox::engine app;

    try {
        // 3. Mount the physical data folder to "mount://core"
        // Note: For this to work, you must have a folder named "core_data"
        // next to your executable containing your "manifest.json".
        app.initialize("/home/jehud/app.zip");

        // 4. Start the synchronous blocking game loop on the main thread
        SANDBOX_RUNNER_RUN(app.ecs);

    } catch (const std::exception& e) {
        // Catch any fatal initialization errors (like a missing manifest file)
        std::cerr << "[Fatal Error in Main]: " << e.what() << '\n';
        return -1;
    }

    return 0;
}