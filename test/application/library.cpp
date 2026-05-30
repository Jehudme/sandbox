#include <iostream>
#include <flecs.h>

// Include your engine's export macros
#include "sandbox/core/plugin.h"

// ============================================================================
// 1. Fake Components
// ============================================================================
struct MockPosition { float x, y; };
struct MockVelocity { float x, y; };
struct MockAudioSource { float volume; };

// ============================================================================
// 2. Fake Sub-Modules
// ============================================================================

// Module A: Physics
struct MockPhysicsModule {
    MockPhysicsModule(flecs::world& world) {
        // Register the module with Flecs
        world.module<MockPhysicsModule>();

        // Register components
        world.component<MockPosition>();
        world.component<MockVelocity>();

        // Create a dummy system to prove execution works

        std::cout << "  -> [Mock] Physics Module loaded!\n";
    }
};
SANDBOX_DEFINE_MODULE(MockPhysicsModule, MockPhysics)

// Module B: Audio
struct MockAudioModule {
    MockAudioModule(flecs::world& world) {
        world.module<MockAudioModule>();
        world.component<MockAudioSource>();
        std::cout << "  -> [Mock] Audio Module loaded!\n";
    }
};
SANDBOX_DEFINE_MODULE(MockAudioModule, MockAudio)

// ============================================================================
// 3. The Master Entry Point
// ============================================================================

struct MockMasterLibrary {
    MockMasterLibrary(flecs::world& world) {
        world.module<MockMasterLibrary>();

        std::cout << "[Mock Library] Mounting Master Entry Point...\n";

        // Statically import the sub-modules into this master block
        world.import<MockPhysicsModule>();
        world.import<MockAudioModule>();

        std::cout << "[Mock Library] Fully Loaded!\n";
    }
};

// Expose the master entry point using the exact anchor your engine looks for
SANDBOX_DEFINE_LIBRARY(MockMasterLibrary)