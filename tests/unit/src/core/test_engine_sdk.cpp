#include <catch2/catch_all.hpp>
#include "sandbox/sdk/engine.hpp"
#include "sandbox/sdk/properties.hpp"

using namespace sandbox;

TEST_CASE("Engine SDK Wrapper", "[engine][sdk]") {
    SECTION("Construct and destroy engine without memory leaks") {
        REQUIRE_NOTHROW([]() {
            Engine engine;
            REQUIRE(engine.get_raw() != nullptr);
        }());
    }

    SECTION("Move semantics transfer ownership") {
        Engine e1;
        Engine e2(std::move(e1));
        REQUIRE(e1.get_raw() == nullptr);
        REQUIRE(e2.get_raw() != nullptr);

        Engine e3;
        e3 = std::move(e2);
        REQUIRE(e2.get_raw() == nullptr);
        REQUIRE(e3.get_raw() != nullptr);
    }

    SECTION("Initialization triggers logic") {
        Properties props;
        props.set_int64("engine/version", 1);
        
        Engine engine;
        REQUIRE(engine.initialize(props));
        REQUIRE(engine.get_ecs() != nullptr);
    }
}
