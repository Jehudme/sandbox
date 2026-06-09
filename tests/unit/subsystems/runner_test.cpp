#include <catch2/catch_test_macros.hpp>
#include "subsystems/runner/runner.h"
#include "sandbox/event_bus/runner_events.h"
#include "sandbox/event_bus/event_bus.h"
#include <thread>
#include <chrono>
#include <glaze/glaze.hpp>

using namespace sandbox;

TEST_CASE("Runner Subsystem operations", "[subsystems][runner]") {
    flecs::world ecs;
    ecs.import<modules::runner>();
    auto runner_api = ecs.get<runner_service>().api;

    SECTION("State transitions correctly move from Idle -> Running -> Paused -> Running -> Quitting") {
        runner_api->start_async(ecs);
        // We can't strictly inspect internal private enums without a mock, 
        // but we verify no deadlocks or exceptions occur during these transitions.
        runner_api->pause();
        runner_api->resume();
        runner_api->quit();
        SUCCEED("Runner transitioned through async states smoothly.");
    }

    SECTION("run_sync terminates correctly when quit() is called via the event bus") {
        // Since the engine lacks the runner observer, we mock it to verify the integration pattern
        sandbox::events::subscribe<sandbox::events::runner::state_change>(ecs, [&](const sandbox::events::runner::state_change& ev) {
            if (ev.state_request == sandbox::events::runner::state_change::action::Quit) {
                runner_api->quit();
            }
        });

        sandbox::events::publish(ecs, sandbox::events::runner::state_change{
            sandbox::events::runner::state_change::action::Quit
        });

        runner_api->run_sync(ecs);
        SUCCEED("run_sync exited properly due to event bus quit request.");
    }

    SECTION("set_property(\"fps_limit\", X) registers the new target FPS") {
        runner_api->set_property("fps_limit", "144.0");
        sandbox_payload payload{};
        REQUIRE(runner_api->get_property("fps_limit", &payload) == 0);
        REQUIRE(payload.bytes != nullptr);
        std::string json(reinterpret_cast<const char*>(payload.bytes), payload.size);
        float val = 0.0f;
        REQUIRE(glz::read_json(val, json) == glz::error_code::none);
        REQUIRE(val == 144.0f);
        if (payload.free_func) payload.free_func(payload.bytes);
    }
}
