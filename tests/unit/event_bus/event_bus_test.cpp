#include <catch2/catch_test_macros.hpp>
#include "sandbox/event_bus/event_bus.h"
#include "sandbox/core/ecs.h"
#include <cstring>

struct test_request { sandbox::abi::flatbuffer_payload req_payload; };
struct test_response { sandbox::abi::flatbuffer_payload res_payload; };

TEST_CASE("Event Bus operations", "[event_bus]") {
    flecs::world ecs;
    uint64_t event_id = ecs.entity().id();

    SECTION("Synchronous publish triggers a registered subscribe observer immediately") {
        bool triggered = false;
        sandbox::events::subscribe_raw(ecs, event_id, [&](const sandbox::abi::flatbuffer_payload& ev) {
            triggered = true;
        });

        sandbox::abi::flatbuffer_payload payload{nullptr, 0, nullptr};
        sandbox::events::publish_raw(ecs, event_id, payload);
        REQUIRE(triggered == true);
    }

    SECTION("Payload data passes correctly to the subscriber") {
        int received = 0;
        sandbox::events::subscribe_raw(ecs, event_id, [&](const sandbox::abi::flatbuffer_payload& ev) {
            if (ev.size == sizeof(int)) {
                std::memcpy(&received, ev.bytes, sizeof(int));
            }
        });

        int val = 99;
        sandbox::abi::flatbuffer_payload payload{reinterpret_cast<uint8_t*>(&val), sizeof(int), nullptr};
        sandbox::events::publish_raw(ecs, event_id, payload);
        REQUIRE(received == 99);
    }

    SECTION("Ensure subscribers restricted to specific flecs::entity channels only receive targeted events") {
        flecs::entity channel_a = ecs.entity();
        flecs::entity channel_b = ecs.entity();

        int a_count = 0, b_count = 0, global_count = 0;

        sandbox::events::subscribe_raw(ecs, event_id, [&](const sandbox::abi::flatbuffer_payload&) { a_count++; }, channel_a);
        sandbox::events::subscribe_raw(ecs, event_id, [&](const sandbox::abi::flatbuffer_payload&) { b_count++; }, channel_b);
        sandbox::events::subscribe_raw(ecs, event_id, [&](const sandbox::abi::flatbuffer_payload&) { global_count++; });

        sandbox::abi::flatbuffer_payload payload{nullptr, 0, nullptr};

        // Publish strictly to channel A
        sandbox::events::publish_raw(ecs, event_id, payload, channel_a);
        
        // Channel A subscriber gets it. Global and B do not.
        REQUIRE(a_count == 1);
        REQUIRE(b_count == 0);
        
        // Publish to global
        sandbox::events::publish_raw(ecs, event_id, payload);
        
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
                // Parse payload as two ints for test
                int a = 0, b = 0;
                if (req.req_payload.size == 2 * sizeof(int)) {
                    std::memcpy(&a, req.req_payload.bytes, sizeof(int));
                    std::memcpy(&b, req.req_payload.bytes + sizeof(int), sizeof(int));
                }
                int sum = a + b;
                uint8_t* out_bytes = new uint8_t[sizeof(int)];
                std::memcpy(out_bytes, &sum, sizeof(int));
                
                sandbox::abi::flatbuffer_payload res{
                    out_bytes,
                    sizeof(int),
                    [](void* p) { delete[] static_cast<uint8_t*>(p); }
                };
                e.set<test_response>({res});
            });

        int inputs[2] = {10, 20};
        sandbox::abi::flatbuffer_payload req_payload{
            reinterpret_cast<uint8_t*>(inputs),
            2 * sizeof(int),
            nullptr
        };

        flecs::entity req_ent = ecs.entity().set<test_request>({req_payload});
        
        REQUIRE(req_ent.has<test_response>());
        auto res = req_ent.get<test_response>();
        int sum = 0;
        std::memcpy(&sum, res->res_payload.bytes, sizeof(int));
        REQUIRE(sum == 30);
        
        if (res->res_payload.free_func) {
            res->res_payload.free_func(res->res_payload.bytes);
        }
        
        req_ent.destruct();
    }
}
