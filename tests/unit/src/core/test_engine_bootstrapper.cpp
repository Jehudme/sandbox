#include <catch2/catch_all.hpp>
#include "core/engine.h"
#include "../../../../sandbox/include/sandbox/abi/engine.h"
#include "sandbox/abi/bootstrapper.h"
#include "core/bootstrapper.h"
#include <string>

using namespace sandbox::core;

TEST_CASE("Engine Bootstrapper C API", "[engine][bootstrapper]") {
    bootstrapper_t::reset();

    properties_t props;
    props.set<std::vector<std::string>>({"engine", "sandbox"}, {});

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

TEST_CASE("Engine Auto-Bootstrapping Sequence", "[engine][bootstrapper][auto]") {
    bootstrapper_t::reset();
    static bool init_called = false;
    init_called = false;

    sandbox_module_info_t m1{};
    m1.name = "AutoMod"; m1.architecture = "test::sys";
    m1.version_major = 1; m1.version_minor = 0; m1.version_patch = 0;
    m1.service = nullptr; m1.requirements = nullptr; m1.requirement_count = 0;
    m1.init_fn = [](ecs_world_t*) { init_called = true; };

    bootstrapper_t::stage_module(m1);

    properties_t props;
    props.set<std::vector<std::string>>({"engine", "sandbox"}, {"test::sys-AutoMod@1.0.0"});

    engine_t engine;
    REQUIRE_NOTHROW(engine.initialize(props));

    REQUIRE(init_called == true);
    bootstrapper_t::reset();
}

TEST_CASE("Engine C-ABI Wrapper", "[engine][c_abi]") {
    bootstrapper_t::reset();

    SECTION("create and destroy") {
        sandbox_engine_t* engine = sandbox_engine_create();
        REQUIRE(engine != nullptr);
        
        // Before initialization, ECS is null or unpopulated
        void* ecs = sandbox_engine_get_ecs(engine);
        // Depending on implementation, ECS might be populated but let's test initialization
        
        sandbox_properties_t* props = sandbox_properties_create();
        bool success = sandbox_engine_initialize(engine, props);
        REQUIRE(success == true);
        
        ecs = sandbox_engine_get_ecs(engine);
        REQUIRE(ecs != nullptr);
        
        sandbox_properties_destroy(props);
        sandbox_engine_destroy(engine);
    }
    
    SECTION("handles null pointers gracefully") {
        REQUIRE(sandbox_engine_initialize(nullptr, nullptr) == false);
        REQUIRE(sandbox_engine_get_ecs(nullptr) == nullptr);
        REQUIRE_NOTHROW(sandbox_engine_destroy(nullptr));
    }
    
    bootstrapper_t::reset();
}
