#include <catch2/catch_all.hpp>
#include "sandbox/sdk/argument.hpp"
#include "sandbox/sdk/engine.hpp"
#include "sandbox/sdk/properties.hpp"

using namespace sandbox;

TEST_CASE("Argument SDK Wrapper", "[argument][sdk]") {
    Properties init_props;
    init_props.set_string("arg_str", "hello");
    init_props.set_int64("arg_int", 42);

    Engine engine;
    REQUIRE(engine.initialize(init_props));

    auto* ecs = static_cast<ecs_world_t*>(engine.get_ecs());

    SECTION("Retrieves values via Argument SDK") {
        REQUIRE(Argument::has(ecs, "arg_str"));
        
        std::string s;
        REQUIRE(Argument::get_string(ecs, "arg_str", s));
        REQUIRE(s == "hello");

        int64_t i = 0;
        REQUIRE(Argument::get_int64(ecs, "arg_int", i));
        REQUIRE(i == 42);
    }

    SECTION("Retrieves keys via Argument SDK") {
        auto keys = Argument::keys(ecs, "");
        REQUIRE(keys.size() >= 2);
    }

    SECTION("Retrieves subtree via Argument SDK") {
        Properties sub = Argument::get_subtree(ecs, "");
        REQUIRE(sub.has("arg_str"));
        REQUIRE(sub.has("arg_int"));
        
        std::string s;
        REQUIRE(sub.get_string("arg_str", s));
        REQUIRE(s == "hello");
    }
}
