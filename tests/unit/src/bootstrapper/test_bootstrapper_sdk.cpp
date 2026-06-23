#include <catch2/catch_all.hpp>
#include "sandbox/sdk/bootstrapper.hpp"
#include "sandbox/sdk/engine.hpp"
#include "sandbox/sdk/properties.hpp"

using namespace sandbox;

TEST_CASE("bootstrapper SDK Wrapper", "[bootstrapper][sdk]") {
    properties props;
    engine engine;
    REQUIRE(engine.initialize(props));

    auto* ecs = static_cast<ecs_world_t*>(engine.get_ecs());

    SECTION("bootstrapper wrappers execution") {
        bootstrapper b(ecs);
        REQUIRE(b.get_raw() != nullptr);
        
        // Boot execution
        REQUIRE_NOTHROW(b.boot(ecs));
        
        // Activating unknown strings logs but doesn't throw across ABI (caught internally or ignored)
        REQUIRE_NOTHROW(b.activate("test::sys-Unknown@1.0.0"));
    }
}
