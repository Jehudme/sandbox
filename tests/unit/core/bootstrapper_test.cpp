#include <catch2/catch_test_macros.hpp>
#include "sandbox/core/bootstrapper.h"
#include "sandbox/core/module_info.h"

using namespace sandbox;

struct mock_a { mock_a(flecs::world&) {} };
struct mock_b { mock_b(flecs::world&) {} };

TEST_CASE("Bootstrapper & Dependency Graph", "[core][bootstrapper]") {
    flecs::world ecs;
    ecs.import<sandbox::bootstrapper>();
    auto& boot = ecs.get_mut<sandbox::bootstrapper>();

    SECTION("Flat Load: A module with zero requirements activates and imports successfully") {
        auto mod = create_module_info<mock_a>("mod_a", 1, 0, {});
        boot.stage({mod});
        REQUIRE_NOTHROW(boot.activate("mod_a"));
        REQUIRE_NOTHROW(boot.execute(ecs));
        REQUIRE(ecs.has<mock_a>());
    }

    SECTION("Linear Dependency: Module A requires Service B. Service B is provided by Module B. Both activate, Module B boots first") {
        auto mod_b = create_module_info<mock_b>("mod_b", 1, 0, {}, "service_b");
        auto mod_a = create_module_info<mock_a>("mod_a", 1, 0, {
            {requirement::kind::service, requirement::strictness::require, "service_b", 1, 0}
        });
        boot.stage({mod_a, mod_b});
        REQUIRE_NOTHROW(boot.activate("mod_a")); // Activating A should auto-resolve and activate B
        REQUIRE_NOTHROW(boot.execute(ecs));
        REQUIRE(ecs.has<mock_b>());
        REQUIRE(ecs.has<mock_a>());
    }

    SECTION("Missing Dependency: Module A requires Service C. Service C is nowhere. Bootstrapper throws std::runtime_error") {
        auto mod_a = create_module_info<mock_a>("mod_a", 1, 0, {
            {requirement::kind::service, requirement::strictness::require, "service_c", 1, 0}
        });
        boot.stage({mod_a});
        REQUIRE_NOTHROW(boot.activate("mod_a"));
        REQUIRE_THROWS_AS(boot.execute(ecs), std::runtime_error);
    }

    SECTION("Semantic Versioning: Module requires Service v2.0. Staged module provides v1.0. Bootstrapper rejects it and throws") {
        auto mod_b = create_module_info<mock_b>("mod_b", 1, 0, {}, "service_b");
        auto mod_a = create_module_info<mock_a>("mod_a", 1, 0, {
            {requirement::kind::service, requirement::strictness::require, "service_b", 2, 0}
        });
        boot.stage({mod_a, mod_b});
        REQUIRE_NOTHROW(boot.activate("mod_a"));
        REQUIRE_THROWS_AS(boot.execute(ecs), std::runtime_error);
    }

    SECTION("Deadlock/Circular Dependency: Module A requires B. Module B requires A. Bootstrapper detects deadlock and throws") {
        auto mod_b = create_module_info<mock_b>("mod_b", 1, 0, {
            {requirement::kind::module, requirement::strictness::require, "mod_a", 1, 0}
        });
        auto mod_a = create_module_info<mock_a>("mod_a", 1, 0, {
            {requirement::kind::module, requirement::strictness::require, "mod_b", 1, 0}
        });
        boot.stage({mod_a, mod_b});
        REQUIRE_NOTHROW(boot.activate("mod_a"));
        REQUIRE_THROWS_AS(boot.execute(ecs), std::runtime_error);
    }

    SECTION("Clean Failure: Activating an unknown module name throws immediately") {
        REQUIRE_THROWS_AS(boot.activate("nonexistent_module"), std::runtime_error);
    }
}
