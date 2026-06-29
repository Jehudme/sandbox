#include <catch2/catch_all.hpp>
#include "sandbox/sdk/engine.hpp"
#include "sandbox/sdk/properties.hpp"

using namespace sandbox;

TEST_CASE("engine SDK Wrapper", "[engine][sdk]") {
    SECTION("Construct and destroy engine without memory leaks") {
        REQUIRE_NOTHROW([]() {
            engine engine;
            REQUIRE(engine.get_raw() != nullptr);
        }());
    }

    SECTION("Move semantics transfer ownership") {
        engine e1;
        engine e2(std::move(e1));
        REQUIRE(e1.get_raw() == nullptr);
        REQUIRE(e2.get_raw() != nullptr);

        engine e3;
        e3 = std::move(e2);
        REQUIRE(e2.get_raw() == nullptr);
        REQUIRE(e3.get_raw() != nullptr);
    }

    SECTION("Initialization triggers logic") {
        properties props;
        props.set("engine/version", 1LL);
        
        engine engine;
        REQUIRE(engine.initialize(props));
        REQUIRE(engine.get_ecs() != nullptr);
    }
}
