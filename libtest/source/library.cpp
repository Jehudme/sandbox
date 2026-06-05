#include <iostream>
#include <flecs.h>
#include "sandbox/core/plugin.h"
#include "sandbox/core/service.h"

// ============================================================================
// 0. DECLARE THE SERVICES (Bridges the string requirements with C++ structs)
// ============================================================================

struct i_logger {};
struct i_window {};
struct i_filesystem {};

SANDBOX_DECLARE_SERVICE(logger_service, i_logger)
SANDBOX_DECLARE_SERVICE(window_service, i_window)
SANDBOX_DECLARE_SERVICE(filesystem_service, i_filesystem)


// ============================================================================
// 1. MODULE DEFINITIONS (The C++ Logic)
// ============================================================================

struct test_math_module {
    test_math_module(flecs::world& ecs) {
        std::cout << "  -> [Math] Booted! (0 Dependencies)\n";
    }
};

struct test_logger_module {
    test_logger_module(flecs::world& ecs) {
        // Register the actual service data into Flecs
        ecs.set<logger_service>({nullptr, 1, 0});
        std::cout << "  -> [Logger] Booted! Provides 'logger_service' (0 Dependencies)\n";
    }
};

struct test_window_module {
    test_window_module(flecs::world& ecs) {
        // Verify we can retrieve the logger service
        logger_service logger = ecs.get_mut<logger_service>();
        std::cout << "     (Window successfully retrieved logger_service)\n";


        // Register the window service
        ecs.set<window_service>({nullptr, 1, 0});
        std::cout << "  -> [Window] Booted! Provides 'window_service'\n";
    }
};

struct test_filesystem_module {
    test_filesystem_module(flecs::world& ecs) {
        ecs.set<filesystem_service>({nullptr, 1, 0});
        std::cout << "  -> [Filesystem] Booted! Provides 'filesystem_service'\n";
    }
};

struct test_renderer_module {
    test_renderer_module(flecs::world& ecs) {
        std::cout << "  -> [Renderer] Booted! Found Window Service and Math Module!\n";
    }
};

struct test_master_app {
    test_master_app(flecs::world& ecs) {
        std::cout << "\n[Master App] ALL SYSTEMS GO! Engine initialized successfully!\n\n";
    }
};


// ============================================================================
// 2. OUT-OF-ORDER REGISTRATION (To torture-test the Bootstrapper!)
// ============================================================================

// 1. Register Master (Requires Renderer + Filesystem) - SHOULD BOOT LAST!
SANDBOX_DECLARE_MODULE(
    test_master_app, test_master, 1, 0, "",
    {sandbox::requirement::kind::module, sandbox::requirement::strictness::require, "test_renderer", 1, 0},
    {sandbox::requirement::kind::service, sandbox::requirement::strictness::require, "filesystem_service", 1, 0}
);

// 2. Register Renderer (Requires Window + Math)
SANDBOX_DECLARE_MODULE(
    test_renderer_module, test_renderer, 1, 0, "",
    {sandbox::requirement::kind::service, sandbox::requirement::strictness::require, "window_service", 1, 0},
    {sandbox::requirement::kind::module, sandbox::requirement::strictness::require, "test_math", 1, 0}
);

// 3. Register Window (Requires Logger)
SANDBOX_DECLARE_MODULE(
    test_window_module, test_window, 1, 0, "window_service",
    {sandbox::requirement::kind::service, sandbox::requirement::strictness::require, "logger_service", 1, 0}
);

// 4. Register Filesystem (Requires Logger)
SANDBOX_DECLARE_MODULE(
    test_filesystem_module, test_filesystem, 1, 0, "filesystem_service",
    {sandbox::requirement::kind::service, sandbox::requirement::strictness::require, "logger_service", 1, 0}
);

// 5. Register Math (0 Dependencies) - SHOULD BOOT FIRST (Pass 1)
SANDBOX_DECLARE_MODULE(test_math_module, test_math, 1, 0, "");

// 6. Register Logger (0 Dependencies) - SHOULD BOOT FIRST (Pass 1)
SANDBOX_DECLARE_MODULE(test_logger_module, test_logger, 1, 0, "logger_service");


// ============================================================================
// 3. EXPORT THE DLL
// ============================================================================

// Exposes the module array and the Flecs import dummy to the engine
SANDBOX_DECLARE_LIBRARY()