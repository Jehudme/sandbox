#include <catch2/catch_test_macros.hpp>
#include "sandbox/core/bootstrapper.h"
#include <flecs.h>

using namespace sandbox;

struct dummy_module {
    dummy_module(flecs::world& ecs) {}
};

TEST_CASE("Core SemVer Logic", "[versioning]") {
    flecs::world ecs;
    bootstrapper boot(ecs);

    library_registry reg;

    SECTION("Module providing v1.5.8 successfully satisfies a requirement for v1.5") {
        reg.modules.push_back(create_module_info<dummy_module>("provider", 1, 5, 8, {}));
        reg.modules.push_back(create_module_info<dummy_module>("requester", 1, 0, 0, {
            { requirement::kind::module, requirement::strictness::require, "provider", 1, 5 }
        }));

        boot.stage(reg);
        boot.activate("requester");
        REQUIRE_NOTHROW(boot.execute(ecs));
    }

    SECTION("Module providing v1.4.9 is rejected when requirement asks for v1.5 (Minor mismatch)") {
        reg.modules.push_back(create_module_info<dummy_module>("provider", 1, 4, 9, {}));
        reg.modules.push_back(create_module_info<dummy_module>("requester", 1, 0, 0, {
            { requirement::kind::module, requirement::strictness::require, "provider", 1, 5 }
        }));

        boot.stage(reg);
        boot.activate("requester");
        REQUIRE_THROWS_AS(boot.execute(ecs), std::runtime_error);
    }

    SECTION("Module providing v2.0.0 is rejected when requirement asks for v1.5 (Major mismatch)") {
        reg.modules.push_back(create_module_info<dummy_module>("provider", 2, 0, 0, {}));
        reg.modules.push_back(create_module_info<dummy_module>("requester", 1, 0, 0, {
            { requirement::kind::module, requirement::strictness::require, "provider", 1, 5 }
        }));

        boot.stage(reg);
        boot.activate("requester");
        REQUIRE_THROWS_AS(boot.execute(ecs), std::runtime_error);
    }
}
