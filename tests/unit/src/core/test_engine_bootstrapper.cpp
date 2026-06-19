#include <catch2/catch_all.hpp>
#include "core/engine.h"
#include "sandbox/core/bootstrapper.h"
#include "core/bootstrapper.h"
#include <string>

using namespace sandbox::core;

TEST_CASE("Engine Bootstrapper C API", "[engine][bootstrapper]") {
    bootstrapper_t::reset();

    properties_t props;
    props.set<std::vector<std::string>>({"engine", "modules"}, {});

    engine_t engine;
    engine.initialize(props);
    
    ecs_world_t* ecs = engine.ecs.c_ptr();

    SECTION("sandbox_get_bootstrapper returns valid pointer") {
        sandbox_bootstrapper_t* b = sandbox_get_bootstrapper(ecs);
        REQUIRE(b != nullptr);
    }

    SECTION("sandbox_bootstrapper_activate with valid params does not crash") {
        sandbox_bootstrapper_t* b = sandbox_get_bootstrapper(ecs);
        
        sandbox_module_info_t m1{};
        m1.name = "TestMod"; m1.architecture = "test::sys";
        m1.version_major = 1; m1.version_minor = 0; m1.version_patch = 0;
        bootstrapper_t::stage_module(m1);

        REQUIRE_NOTHROW(sandbox_bootstrapper_activate(b, "test::sys", "TestMod", 1, 0, 0));
    }

    SECTION("sandbox_bootstrapper_activate_string with valid string does not crash") {
        sandbox_bootstrapper_t* b = sandbox_get_bootstrapper(ecs);
        
        sandbox_module_info_t m1{};
        m1.name = "TestModString"; m1.architecture = "test::sys";
        m1.version_major = 1; m1.version_minor = 0; m1.version_patch = 0;
        bootstrapper_t::stage_module(m1);

        REQUIRE_NOTHROW(sandbox_bootstrapper_activate_string(b, "test::sys-TestModString@1.0.*"));
    }

    SECTION("sandbox_bootstrapper_boot does not crash") {
        sandbox_bootstrapper_t* b = sandbox_get_bootstrapper(ecs);
        REQUIRE_NOTHROW(sandbox_bootstrapper_boot(b, ecs));
    }
    
    bootstrapper_t::reset();
}
