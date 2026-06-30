#include <catch2/catch_all.hpp>
#include <flecs.h>
#include "core/engine.h"
#include <sandbox/sdk/configuration.hpp>
#include <sandbox/sdk/logs.hpp>
#include <sandbox/sdk/events.hpp>
#include "core/bootstrapper.h"

using namespace sandbox::core;
using namespace sandbox::modules;

struct TestDamageEvent {
    int damage;
};

TEST_CASE("Events Module", "[events]") {
    bootstrapper_t::reset();

    // Setup properties
    properties_t engine_props;
    engine_props.set<std::vector<std::string>>({"engine", "libraries"}, {"./configuration.so", "./logs.so", "./events.so"});
    engine_props.set<std::vector<std::string>>({"engine", "sandbox"}, {"sandbox::core-configuration@1.0.0", "sandbox::core-logs@1.0.0", "sandbox::core-events@1.0.0"});

    // Initialize engine
    engine_t engine;
    REQUIRE_NOTHROW(engine.initialize(engine_props));

    flecs::world& world = engine.ecs;

    int received_damage = 0;
    
    // Subscribe using the clean C++ SDK
    flecs::entity sub = events::subscribe<TestDamageEvent>(world, [](const TestDamageEvent* e, void* user_data) {
        int* out = static_cast<int*>(user_data);
        *out = e->damage;
    }, &received_damage);
    
    REQUIRE(sub.is_valid());

    // Publish using the clean C++ SDK
    TestDamageEvent damage = {42};
    events::publish(world, &damage);
    
    REQUIRE(received_damage == 42);
    
    // Unsubscribe (destroy the subscription entity)
    sub.destruct();
    
    // Publish again, shouldn't receive
    TestDamageEvent damage2 = {100};
    events::publish(world, &damage2);
    
    REQUIRE(received_damage == 42);

    bootstrapper_t::reset();
}
