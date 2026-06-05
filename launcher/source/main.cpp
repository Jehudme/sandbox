#include <iostream>
#include <filesystem>
#include <exception>
#include <string>

#include <CLI/CLI.hpp>
#include "sandbox/core/engine.h"
#include "sandbox/core/plugin.h"
#include "sandbox/event_bus/runner_events.h"
#include "sandbox/subsystems/runner/irunner.h"

int main(int argc, char* argv[]) {
    CLI::App app{"Sandbox Meta-Engine Runtime Launcher"};
    sandbox::engine::arguments args;
    bool run = false;

    app.add_option("--mount,-m", args.app_mount, "Path to the application archive or directory")
       ->required()
       ->check(CLI::ExistingPath);

    app.add_flag("--dev,-d", args.dev_mode, "Enable developer mode (verbose logging)");
    app.add_flag("--run,-r", run, "Run the engine main loop immediately after boot");

    // Allows passing arbitrary module properties without modifying the launcher.
    // Example: -p Renderer=Vulkan -p Physics.TickRate=120
    app.add_option("--prop,-p", args.module_args, "Custom module properties (Key=Value)");

    CLI11_PARSE(app, argc, argv);

    // Must be called before the first flecs::world is constructed.
    // The OS API override (dlopen/dlsym callbacks) must be in place before
    // any ecs_import_from_library call, which happens during engine::initialize().
    sandbox::configure_plugin_os_api();
    sandbox::engine engine_instance;

    try {
        engine_instance.initialize(args);

        if (run) {
            engine_instance.ecs.get<sandbox::runner_service>().api->run_sync(engine_instance.ecs);
        }

    } catch (const std::exception& fatal_error) {
        // The engine may not yet have a logger; write directly to stderr.
        std::cerr << "[Launcher] Fatal crash: " << fatal_error.what() << '\n';
        return -1;
    }

    return 0;
}