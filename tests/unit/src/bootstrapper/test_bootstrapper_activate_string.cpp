#include <catch2/catch_all.hpp>
#include "../../test_accessor.h"
#include "core/exceptions.h"

using namespace sandbox::core;

TEST_CASE("Bootstrapper string activation parsing", "[bootstrapper][activation]") {
    bootstrapper_test_accessor::reset();
    flecs::world ecs;

    // Setup some fake sandbox for activate to resolve against
    sandbox_module_info_t m1{};
    m1.name = "Renderer"; m1.architecture = "sandbox::system";
    m1.version_major = 1; m1.version_minor = 2; m1.version_patch = 3;
    bootstrapper_t::stage_module(m1);
    
    sandbox_module_info_t m2{};
    m2.name = "Physics"; m2.architecture = "sandbox::system";
    m2.version_major = 2; m2.version_minor = 0; m2.version_patch = 0;
    bootstrapper_t::stage_module(m2);

    bootstrapper_t bootstrapper;

    SECTION("Parses full version correctly") {
        REQUIRE_NOTHROW(bootstrapper.activate(ecs, "sandbox::system-Renderer@1.2.3"));
    }

    SECTION("Parses missing patch with wildcard") {
        REQUIRE_NOTHROW(bootstrapper.activate(ecs, "sandbox::system-Renderer@1.2.*"));
    }

    SECTION("Parses missing minor and patch") {
        REQUIRE_NOTHROW(bootstrapper.activate(ecs, "sandbox::system-Physics@2"));
    }

    SECTION("Throws on invalid string (no @)") {
        REQUIRE_NOTHROW(bootstrapper.activate(ecs, "sandbox::system-Renderer"));
    }

    SECTION("Throws on invalid string (no dash)") {
        REQUIRE_NOTHROW(bootstrapper.activate(ecs, "sandbox::system_Renderer@1.2.3"));
    }

    bootstrapper_test_accessor::reset();
}
