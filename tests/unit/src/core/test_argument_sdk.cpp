#include <catch2/catch_all.hpp>
#include "sandbox/sdk/argument.hpp"
#include "sandbox/sdk/engine.hpp"
#include "sandbox/sdk/properties.hpp"

using namespace sandbox;

TEST_CASE("argument SDK Wrapper", "[argument][sdk]") {
    properties init_props;
    init_props.set_string("arg_str", "hello");
    init_props.set_int64("arg_int", 42);

    engine engine;
    REQUIRE(engine.initialize(init_props));

    auto* ecs = static_cast<ecs_world_t*>(engine.get_ecs());

    SECTION("Retrieves values via argument SDK") {
        REQUIRE(argument::has(ecs, "arg_str"));
        
        std::string s;
        REQUIRE(argument::get_string(ecs, "arg_str", s));
        REQUIRE(s == "hello");

        int64_t i = 0;
        REQUIRE(argument::get_int64(ecs, "arg_int", i));
        REQUIRE(i == 42);
    }

    SECTION("Retrieves keys via argument SDK") {
        auto keys = argument::keys(ecs, "");
        REQUIRE(keys.size() >= 2);
    }

    SECTION("Retrieves subtree via argument SDK") {
        properties sub = argument::get_subtree(ecs, "");
        REQUIRE(sub.has("arg_str"));
        REQUIRE(sub.has("arg_int"));
        
        std::string s;
        REQUIRE(sub.get_string("arg_str", s));
        REQUIRE(s == "hello");
    }
}
