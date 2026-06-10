#include <iostream>
#include <flecs.h>
#include <sandbox/core/plugin.h>
#include <sandbox/api/logger_api.h>
#include <sandbox/api/filesystem_api.h>
#include <sandbox/api/runner_api.h>
#include <sandbox/utilities/events.h>

// 1. Define a dummy event for the Event Bus
struct my_dummy_event {
    int secret_value;
};

// 2. Define a custom component for ECS
struct rotation_component {
    float angle;
};

// 3. Define the Plugin Module (The Core Logic)
struct showcase_plugin_module {
    showcase_plugin_module(flecs::world& ecs) {
        
        // --- API RETRIEVAL ---
        // Safely retrieve the subsystem wrappers from the ECS
        sandbox::sdk::logger logger(ecs);
        sandbox::sdk::filesystem vfs(ecs);
        sandbox::sdk::runner runner(ecs);

        // --- LOGGING ---
        logger.log(2, "[Showcase Plugin] Successfully booted and retrieved all APIs!");

        // --- FILESYSTEM / CONFIG ---
        // Read a dummy manifest or config file using the VFS
        auto manifest_res = vfs.read_text("mount://app/manifest.json");
        if (manifest_res) {
            logger.log(2, "[Showcase Plugin] Manifest loaded successfully! Length: " + std::to_string(manifest_res->size()));
        } else {
            logger.log(3, "[Showcase Plugin] Could not find manifest.json, but VFS is working!");
        }

        // --- EVENT BUS ---
        // Register an event listener for our custom event
        sandbox::events::subscribe<my_dummy_event>(ecs, [](const my_dummy_event& ev) {
            std::cout << "  [Event Bus] Received my_dummy_event with value: " << ev.secret_value << "\n";
        });

        // Publish the dummy event
        sandbox::events::publish(ecs, my_dummy_event{42});

        // --- ECS INTEGRATION ---
        // Register the custom component
        ecs.component<rotation_component>();

        // Create a dummy entity
        auto e = ecs.entity("ShowcaseEntity").set<rotation_component>({0.0f});

        // Register a Flecs system that executes a tick
        ecs.system<rotation_component>("ShowcaseSystem")
            .each([](flecs::entity e, rotation_component& rot) {
                rot.angle += 0.016f;
                if (rot.angle < 0.032f) {
                    std::cout << "  [ECS System] Ticking ShowcaseEntity, angle is now " << rot.angle << "\n";
                }
            });
            
        logger.log(2, "[Showcase Plugin] Initialization complete!");
    }
};

// ============================================================================
// MODULE REGISTRATION
// ============================================================================

// Register the module with the bootstrapper, declaring its dependencies.
// The engine will guarantee that core_logger, core_vfs, and core_runner are ready.
SANDBOX_DECLARE_MODULE(showcase_plugin_module, showcase_plugin, 1, 0, 0, "",
    {sandbox::requirement::kind::service, sandbox::requirement::strictness::require, "logger_service", 1, 0},
    {sandbox::requirement::kind::service, sandbox::requirement::strictness::require, "filesystem_service", 1, 0},
    {sandbox::requirement::kind::service, sandbox::requirement::strictness::require, "runner_service", 1, 0}
);

// ============================================================================
// DLL ENTRY POINT
// ============================================================================
SANDBOX_DECLARE_LIBRARY()