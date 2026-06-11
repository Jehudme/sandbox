#include <catch2/catch_test_macros.hpp>
#include "modules/logger/logger.h"
#include <sandbox/modules/logger/logger_api.h>
#include <glaze/glaze.hpp>

using namespace sandbox;

TEST_CASE("Logger Subsystem operations", "[subsystems][logger]") {
    flecs::world ecs;
    ecs.import<modules::logger>();
    auto logger_api = sdk::logger(ecs);

    SECTION("Trace/Debug logs are ignored if dev_mode is false and level is Info") {
        logger_api.set_property("logger_level", std::string("2")); // spdlog::level::info is 2
        // By default spdlog will just drop it without error.
        // We assert that the macro executes cleanly without crashing or throwing.
        SANDBOX_TRACE(ecs, "Trace test {}", 42);
        SANDBOX_DEBUG(ecs, "Debug test {}", 42);
    }

    SECTION("SANDBOX_ERROR_THROW correctly returns -1 regardless of engine config") {
        int32_t result = 0;
        flatbuffers::FlatBufferBuilder builder;
        auto msg = builder.CreateString("forced error");
        auto file = builder.CreateString("file");
        sandbox::schemas::logger::LogArgsBuilder lmb(builder);
        lmb.add_level(sandbox::schemas::logger::LogLevel_Error);
        lmb.add_message(msg);
        lmb.add_source_file(file);
        lmb.add_source_line(1);
        lmb.add_throw_on_error(true);
        lmb.add_out_result_ptr(reinterpret_cast<uint64_t>(&result));
        builder.Finish(lmb.Finish());

        auto* svc = ecs.try_get<logger_service>();
        svc->execute_command(svc->instance, static_cast<uint32_t>(sandbox::schemas::logger::LoggerCommand_Log), builder.GetBufferPointer(), builder.GetSize());
        REQUIRE(result == -1);
    }

    SECTION("Setting dynamic properties updates internal filtering immediately") {
        logger_api.set_property("logger_level", 1); // spdlog::level::debug is 1
        auto res = logger_api.get_property<int>("logger_level");
        REQUIRE(res.has_value());
        REQUIRE(res.value() == 1);
    }
}
