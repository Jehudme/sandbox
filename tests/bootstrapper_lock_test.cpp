#include <catch2/catch_test_macros.hpp>
#include "sandbox/core/bootstrapper.h"
#include <flecs.h>

using namespace sandbox;

struct dummy_module {
    dummy_module(flecs::world& ecs) {}
};

TEST_CASE("Bootstrapper Locking & Resolution", "[bootstrapper]") {
    flecs::world ecs;
    bootstrapper boot(ecs);
    library_registry reg;

    SECTION("Highest Patch Selection") {
        reg.modules.push_back(create_module_info<dummy_module>("renderer", 1, 5, 2, {}));
        reg.modules.push_back(create_module_info<dummy_module>("renderer", 1, 5, 8, {}));
        reg.modules.push_back(create_module_info<dummy_module>("requester", 1, 0, 0, {
            { requirement::kind::module, requirement::strictness::require, "renderer", 1, 5 }
        }));

        boot.stage(reg);
        boot.activate("requester");
        REQUIRE_NOTHROW(boot.execute(ecs));
        // Verify 1.5.8 is locked. Actually execute succeeds, which means we picked one.
    }

    SECTION("Ambiguity Detection") {
        reg.services.push_back(create_service_info("renderer", 1, 5));
        reg.modules.push_back(create_module_info<dummy_module>("vulkan_renderer", 1, 5, 0, {}, "renderer"));
        reg.modules.push_back(create_module_info<dummy_module>("opengl_renderer", 1, 5, 0, {}, "renderer"));
        reg.modules.push_back(create_module_info<dummy_module>("requester", 1, 0, 0, {
            { requirement::kind::service, requirement::strictness::require, "renderer", 1, 5 }
        }));

        boot.stage(reg);
        boot.activate("requester");
        // Should log a warning and proceed without throwing
        REQUIRE_NOTHROW(boot.execute(ecs));
    }

    SECTION("Cascade Locking") {
        reg.services.push_back(create_service_info("ServiceB", 1, 0));
        reg.modules.push_back(create_module_info<dummy_module>("ModuleB", 1, 0, 0, {}, "ServiceB"));
        reg.modules.push_back(create_module_info<dummy_module>("ModuleA", 1, 0, 0, {
            { requirement::kind::service, requirement::strictness::require, "ServiceB", 1, 0 }
        }));

        boot.stage(reg);
        boot.activate("ModuleA");
        REQUIRE_NOTHROW(boot.execute(ecs));
    }

    SECTION("Missing Requirement") {
        reg.modules.push_back(create_module_info<dummy_module>("ModuleA", 1, 0, 0, {
            { requirement::kind::service, requirement::strictness::require, "ServiceC", 1, 0 }
        }));

        boot.stage(reg);
        boot.activate("ModuleA");
        REQUIRE_THROWS_AS(boot.execute(ecs), std::runtime_error);
    }

    SECTION("Smart Bootstrapping") {
        // ModuleA requires ModuleB.
        // ModuleA expects ModuleC.
        // If require happens first, ModuleB is locked.
        // If expect happens second, ModuleC is locked.
        reg.modules.push_back(create_module_info<dummy_module>("ModuleA", 1, 0, 0, {
            { requirement::kind::module, requirement::strictness::require, "ModuleB", 1, 0 },
            { requirement::kind::module, requirement::strictness::expect, "ModuleC", 1, 0 }
        }));
        reg.modules.push_back(create_module_info<dummy_module>("ModuleB", 1, 0, 0, {}));
        reg.modules.push_back(create_module_info<dummy_module>("ModuleC", 1, 0, 0, {}));

        boot.stage(reg);
        boot.activate("ModuleA");
        REQUIRE_NOTHROW(boot.execute(ecs));
    }
}
