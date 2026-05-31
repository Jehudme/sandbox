#include <iostream>
#include <flecs.h>

#include "sandbox/core/plugin.h"
#include "sandbox/utilities/events.h"

struct MockPosition { float x, y; };
struct MockVelocity { float x, y; };
struct MockAudioSource { float volume; };

struct MockCollisionEvent {
    float impact_force;
};

struct MockAudioModule {
    explicit MockAudioModule(flecs::world& world) {
        world.module<MockAudioModule>();
        world.component<MockAudioSource>();

        flecs::entity obs = sandbox::events::subscribe<MockCollisionEvent>(
            world,
            [](const MockCollisionEvent& e) {
                std::cout
                    << "    [Audio Module] Heard a collision! Playing crunch sound at volume "
                    << e.impact_force << "!\n";
            }
        );

        obs.child_of<MockAudioModule>();

        std::cout << "  -> [Mock] Audio Module loaded (Listening for collisions)...\n";
    }
};
SANDBOX_DEFINE_MODULE(MockAudioModule, MockAudio)

struct MockPhysicsModule {
    explicit MockPhysicsModule(flecs::world& world) {
        world.module<MockPhysicsModule>();
        world.component<MockPosition>();
        world.component<MockVelocity>();

        world.system<MockPosition>("SimulatePhysics")
            .each([](flecs::iter& it, size_t /*i*/, MockPosition& /*p*/) {
                std::cout << "    [Physics Module] System running. Simulating collision...\n";
                sandbox::events::publish(it.world(), MockCollisionEvent{85.5f});
            });

        std::cout << "  -> [Mock] Physics Module loaded (Publisher)...\n";
    }
};
SANDBOX_DEFINE_MODULE(MockPhysicsModule, MockPhysics)

struct MockMasterLibrary {
    explicit MockMasterLibrary(flecs::world& world) {
        world.module<MockMasterLibrary>();

        std::cout << "[Mock Library] Mounting Master Entry Point...\n";

        world.import<MockAudioModule>();
        world.import<MockPhysicsModule>();

        world.entity("DummyCollider").set<MockPosition>({10.0f, 20.0f});

        std::cout << "[Mock Library] Fully Loaded!\n";
    }
};
SANDBOX_DEFINE_LIBRARY(MockMasterLibrary)