#include <catch2/catch_test_macros.hpp>
#include "sandbox/event_bus/event_bus.h"
#include "sandbox/core/ecs.h"

struct test_event { int payload; };

struct test_request { int a; int b; };
struct test_response { int sum; };

TEST_CASE("Event Bus operations", "[event_bus]") {
    flecs::world ecs;

    SECTION("Synchronous publish triggers a registered subscribe observer immediately") {
        bool triggered = false;
        sandbox::events::subscribe<test_event>(ecs, [&](const test_event& ev) {
            triggered = true;
        });

        sandbox::events::publish(ecs, test_event{42});
        REQUIRE(triggered == true);
    }

    SECTION("Payload data passes correctly to the subscriber") {
        int received = 0;
        sandbox::events::subscribe<test_event>(ecs, [&](const test_event& ev) {
            received = ev.payload;
        });

        sandbox::events::publish(ecs, test_event{99});
        REQUIRE(received == 99);
    }

    SECTION("Ensure subscribers restricted to specific flecs::entity channels only receive targeted events") {
        flecs::entity channel_a = ecs.entity();
        flecs::entity channel_b = ecs.entity();

        int a_count = 0, b_count = 0, global_count = 0;

        sandbox::events::subscribe<test_event>(ecs, [&](const test_event&) { a_count++; }, channel_a);
        sandbox::events::subscribe<test_event>(ecs, [&](const test_event&) { b_count++; }, channel_b);
        sandbox::events::subscribe<test_event>(ecs, [&](const test_event&) { global_count++; });

        // Publish strictly to channel A
        sandbox::events::publish(ecs, test_event{1}, channel_a);
        
        // Channel A subscriber gets it. Global and B do not.
        REQUIRE(a_count == 1);
        REQUIRE(b_count == 0);
        
        // Publish to global
        sandbox::events::publish(ecs, test_event{2});
        
        // Only global subscriber gets it
        REQUIRE(global_count == 1);
        REQUIRE(a_count == 1); // Should remain 1
    }
}

TEST_CASE("ECS Request/Response loop", "[event_bus][request_response]") {
    flecs::world ecs;

    SECTION("Observers can process a request component and append a response component statelessly") {
        ecs.observer<test_request>()
            .event(flecs::OnSet)
            .each([](flecs::entity e, test_request& req) {
                e.set<test_response>({req.a + req.b});
            });

        flecs::entity req_ent = ecs.entity().set<test_request>({10, 20});
        
        REQUIRE(req_ent.has<test_response>());
        REQUIRE(req_ent.get<test_response>()->sum == 30);
        
        req_ent.destruct();
    }
}
