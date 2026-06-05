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
    
    std::filesystem::path app_mount;
    bool dev_mode = false;
    std::unordered_map<std::string, std::string> module_args;
    bool run = false;

    app.add_option("--mount,-m", app_mount, "Path to the application archive or directory")
       ->required()
       ->check(CLI::ExistingPath);

    app.add_flag("--dev,-d", dev_mode, "Enable developer mode (verbose logging)");
    app.add_flag("--run,-r", run, "Run the engine main loop immediately after boot");

    // Allows passing arbitrary module properties without modifying the launcher.
    // Example: -p Renderer=Vulkan -p Physics.TickRate=120
    app.add_option("--prop,-p", module_args, "Custom module properties (Key=Value)");

    CLI11_PARSE(app, argc, argv);

    std::unordered_map<std::string, std::any> config;
    config["app_mount"] = app_mount;
    config["dev_mode"] = dev_mode;
    config["module_args"] = module_args;
    
    // Add default subsystem properties explicitly required by user instructions
    config["logger_level"] = spdlog::level::info;
    config["enable_async"] = true;
    config["fps_limit"] = 60;

    // Must be called before the first flecs::world is constructed.
    // The OS API override (dlopen/dlsym callbacks) must be in place before
    // any ecs_import_from_library call, which happens during engine::initialize().
    sandbox::configure_plugin_os_api();
    sandbox::engine engine_instance;

    try {
        engine_instance.initialize(config);

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