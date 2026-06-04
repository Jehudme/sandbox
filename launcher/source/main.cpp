#include <iostream>
#include <filesystem>
#include <exception>
#include <string>

#include <CLI/CLI.hpp>
#include "sandbox/core/engine.h"
#include "sandbox/core/plugin.h"
#include "sandbox/event_bus/runner_events.h"

int main(int argc, char* argv[]) {
    CLI::App app{"Sandbox Meta-Engine Runtime Launcher"};
    sandbox::engine::arguments args;
    bool run = false;

    // 1. Core Boot Routing
    app.add_option("--mount,-m", args.app_mount, "Mount paths (e.g., --mount application=test-app.zip)")
       ->required()
       ->check(CLI::ExistingPath);

    app.add_flag("--dev,-d", args.dev_mode, "Enable developer mode layers");
    app.add_flag("--run,-r", run,"Run the engine immediately after boot");

    // 2. The Dynamic Module Payload
    // This allows users to pass infinite custom arguments without modifying the launcher!
    // Example: -p Renderer=Vulkan -p Physics.TickRate=120
    app.add_option("--prop,-p", args.module_args, "Custom module properties (Key=Value)");

    // 3. Execution Handoff
    CLI11_PARSE(app, argc, argv);

    sandbox::configure_plugin_os_api();
    sandbox::engine engine_instance;

    try {
        std::cout << "[Launcher] Booting engine core...\n";
        engine_instance.initialize(args);

        if (run) SANDBOX_RUNNER_RUN(engine_instance.ecs);

    } catch (const std::exception& fatal_error) {
        std::cerr << "\n[Fatal Core Crash Caught]: " << fatal_error.what() << '\n';
        return -1;
    }

    return 0;
}