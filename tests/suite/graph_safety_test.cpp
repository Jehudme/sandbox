#include <catch2/catch_test_macros.hpp>
#include "sandbox/core/bootstrapper.h"
#include <flecs.h>

using namespace sandbox;

struct mock_module_a { mock_module_a(flecs::world& ecs) {} };
struct mock_module_b { mock_module_b(flecs::world& ecs) {} };
struct mock_module_c { mock_module_c(flecs::world& ecs) {} };

TEST_CASE("Bootstrapper Graph Safety", "[bootstrapper][graph]") {
    library_registry reg;

    SECTION("Major Version Collision throws std::runtime_error") {
        flecs::world ecs;
        bootstrapper boot(ecs);

        // Module A requires Service X v1.0
        reg.modules.push_back(create_module_info<mock_module_a>("module_a", 1, 0, 0, {
            { requirement::kind::service, requirement::strictness::require, "service_x", 1, 0 }
        }));

        // Module B requires Service X v2.0
        reg.modules.push_back(create_module_info<mock_module_b>("module_b", 1, 0, 0, {
            { requirement::kind::service, requirement::strictness::require, "service_x", 2, 0 }
        }));

        // We provide both versions of Service X
        reg.services.push_back(create_service_info("service_x", 1, 0));
        reg.services.push_back(create_service_info("service_x", 2, 0));

        reg.modules.push_back(create_module_info<mock_module_c>("provider_x1", 1, 0, 0, {}, "service_x"));
        reg.modules.push_back(create_module_info<mock_module_c>("provider_x2", 2, 0, 0, {}, "service_x"));

        boot.stage(reg);
        boot.activate("module_a");
        boot.activate("module_b");

        // The conflict should be caught during execute
        REQUIRE_THROWS_AS(boot.execute(ecs), std::runtime_error);
    }

    SECTION("Circular Dependency is caught by Kahn's Algorithm") {
        flecs::world ecs;
        bootstrapper boot(ecs);

        // Module A requires Module B
        reg.modules.push_back(create_module_info<mock_module_a>("module_a", 1, 0, 0, {
            { requirement::kind::module, requirement::strictness::require, "module_b", 1, 0 }
        }));

        // Module B requires Module A
        reg.modules.push_back(create_module_info<mock_module_b>("module_b", 1, 0, 0, {
            { requirement::kind::module, requirement::strictness::require, "module_a", 1, 0 }
        }));

        boot.stage(reg);
        boot.activate("module_a");
        // We only explicitly activate module_a. module_b gets pulled in via requirement.

        REQUIRE_THROWS_AS(boot.execute(ecs), std::runtime_error);
    }

    SECTION("Unfulfilled Expectation Safety") {
        flecs::world ecs;
        bootstrapper boot(ecs);

        // Module A *expects* Service Y v1.0, but we do NOT provide it
        reg.modules.push_back(create_module_info<mock_module_a>("module_a", 1, 0, 0, {
            { requirement::kind::service, requirement::strictness::expect, "service_y", 1, 0 }
        }));

        boot.stage(reg);
        boot.activate("module_a");

        // Should NOT throw an error. Soft dependency is safely ignored.
        REQUIRE_NOTHROW(boot.execute(ecs));
    }
}
