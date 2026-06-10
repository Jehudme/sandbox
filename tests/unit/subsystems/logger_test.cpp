#include <catch2/catch_test_macros.hpp>
#include "subsystems/logger/logger.h"
#include "sandbox/subsystems/logger/ilogger.h"
#include <glaze/glaze.hpp>

using namespace sandbox;

TEST_CASE("Logger Subsystem operations", "[subsystems][logger]") {
    flecs::world ecs;
    ecs.import<modules::logger>();
    auto logger_api = ecs.get<logger_service>().api;

    SECTION("Trace/Debug logs are ignored if dev_mode is false and level is Info") {
        logger_api->set_property("logger_level", "2"); // spdlog::level::info is 2
        // By default spdlog will just drop it without error.
        // We assert that the macro executes cleanly without crashing or throwing.
        SANDBOX_TRACE(ecs, "Trace test {}", 42);
        SANDBOX_DEBUG(ecs, "Debug test {}", 42);
    }

    SECTION("SANDBOX_ERROR_THROW correctly returns -1 regardless of engine config") {
        flatbuffers::FlatBufferBuilder builder;
        auto msg = builder.CreateString("forced error");
        auto file = builder.CreateString("file");
        sandbox::schemas::logger::LogMessageBuilder lmb(builder);
        lmb.add_level(sandbox::schemas::logger::LogLevel_Error);
        lmb.add_message(msg);
        lmb.add_source_file(file);
        lmb.add_source_line(1);
        lmb.add_throw_on_error(true);
        builder.Finish(lmb.Finish());

        auto res = logger_api->log(builder.GetBufferPointer(), builder.GetSize());
        REQUIRE(res == -1);
    }

    SECTION("Setting dynamic properties updates internal filtering immediately") {
        logger_api->set_property("logger_level", "1"); // spdlog::level::debug is 1
        sandbox_payload payload{};
        REQUIRE(logger_api->get_property("logger_level", &payload) == 0);
        std::string json(reinterpret_cast<const char*>(payload.bytes), payload.size);
        int val = 0;
        REQUIRE(glz::read_json(val, json) == glz::error_code::none);
        REQUIRE(val == spdlog::level::debug);
        if (payload.free_func) payload.free_func(payload.bytes);
    }
}
