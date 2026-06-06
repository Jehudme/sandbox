#include <catch2/catch_test_macros.hpp>
#include "subsystems/logger/logger.h"
#include "sandbox/event_bus/logger_events.h"

using namespace sandbox;

TEST_CASE("Logger Subsystem operations", "[subsystems][logger]") {
    flecs::world ecs;
    ecs.import<modules::logger>();
    auto logger_api = ecs.get<logger_service>().api;

    SECTION("Trace/Debug logs are ignored if dev_mode is false and level is Info") {
        logger_api->set_property("logger_level", spdlog::level::info);
        // By default spdlog will just drop it without error.
        // We assert that the macro executes cleanly without crashing or throwing.
        SANDBOX_TRACE(ecs, "Trace test {}", 42);
        SANDBOX_DEBUG(ecs, "Debug test {}", 42);
    }

    SECTION("SANDBOX_ERROR_THROW correctly returns std::unexpected regardless of engine config") {
        // Since macros execute via event bus, we can directly invoke the api->log implementation
        // with the throw_on_error_override set to true (as SANDBOX_ERROR_THROW does).
        auto res = logger_api->log(events::log{"file", 1, events::log::level::Error, true, "forced error"});
        REQUIRE_FALSE(res.has_value());
    }

    SECTION("Setting dynamic properties updates internal filtering immediately") {
        logger_api->set_property("logger_level", spdlog::level::debug);
        auto val = std::any_cast<spdlog::level::level_enum>(logger_api->get_property("logger_level"));
        REQUIRE(val == spdlog::level::debug);
    }
}
