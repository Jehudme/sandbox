#include <iostream>
#include <flecs.h>

// ============================================================================
// 1. Data Structures & Event Payloads
// ============================================================================
struct GlobalBusTag {};

struct Position {
    float x, y;
};

struct MockCollisionEvent {
    float impact_force;
};

// ============================================================================
// 2. Main Simulation Execution
// ============================================================================
int main() {
    flecs::world world;

    std::cout << "[Init] Allocating Central Channel Entity...\n";
    // 1. Create the central bus entity and give it the routing tag
    flecs::entity central_bus = world.entity("GlobalEventBus").add<GlobalBusTag>();

    std::cout << "[Init] Binding Decoupled Event Observer...\n";

    // 2. SUBSCRIBER: Listen for MockCollisionEvent, but ONLY if it is routed via GlobalBusTag
    world.observer()
        .event<MockCollisionEvent>()
        .with<GlobalBusTag>()
        .run([](flecs::iter& it) {
            // Unpack the payload pointer safely
            const auto* payload = static_cast<const MockCollisionEvent*>(it.param());
            if (payload) {
                std::cout << "    [Audio Listener Intercept] Caught event! crunch sound volume: "
                          << payload->impact_force << "!\n";
            }
        });

    std::cout << "[Init] Registering System Pipeline...\n";

    // 3. PUBLISHER: Physics system that triggers the event
    world.system<Position>("SimulatePhysics")
        .each([central_bus](flecs::iter& it, size_t index, Position& p) {
            std::cout << "    [Physics Publisher] Step complete. Emitting collision notification...\n";

            MockCollisionEvent event_data{ 85.5f };

            // Emits the typed event instance ON the bus entity, actively tagged with GlobalBusTag
            it.world().event<MockCollisionEvent>()
                .entity(central_bus)
                .id<GlobalBusTag>() // <-- This is the magic link!
                .ctx(event_data)
                .emit();
        });

    // Spawn a dummy collider so the physics system actually ticks
    world.entity("DummyCollider").set<Position>({ 10.0f, 20.0f });

    std::cout << "\n--------------------------------------------------------\n";
    std::cout << "[Simulation] Advancing world timeline 1 frame...\n";

    world.progress();

    std::cout << "--------------------------------------------------------\n";
    return 0;
}