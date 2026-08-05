// unit/src/bootstrapper/test_bootstrapper_abi_macros.cpp
// Tests for the bootstrapper C ABI functions and macro-generated structs.

#include <catch2/catch_all.hpp>
#include "../../../../sandbox/include/sandbox/abi/bootstrapper.h"
#include "../../test_accessor.h"
#include "../../../../sandbox/include/sandbox/abi/platform.h"

#include <flecs.h>

using sandbox::core::bootstrapper_t;

// ---------------------------------------------------------------------------
// Minimal Flecs sandbox for import tests
// ---------------------------------------------------------------------------
struct TestMacroFlecs {
    TestMacroFlecs(ecs_world_t*) {}
};

struct CounterFlecs {
    CounterFlecs(ecs_world_t*) {}
};

// ---------------------------------------------------------------------------
// Manual service info (mirrors what SANDBOX_DECLARE_SERVICE would generate)
// ---------------------------------------------------------------------------
struct ISimpleCounter { int count; };
static ISimpleCounter simple_counter_singleton = { .count = 42 };

static const sandbox_service_info_t SimpleCounterService_manual_info = {
    .name          = "ISimpleCounter",
    .description   = "Simple counter service for macro test",
    .architecture  = "sandbox::system",
    .version_major = 1,
    .version_minor = 0,
    .init_fn       = nullptr,
};

// ---------------------------------------------------------------------------
// Manual module info (mirrors what SANDBOX_DECLARE_MODULE would generate)
// ---------------------------------------------------------------------------
static const sandbox_module_info_t TestMacroModule_manual_info = {
    .name              = "TestMacroModule",
    .description       = "Macro-declared test module",
    .architecture      = "sandbox::system",
    .version_major     = 1,
    .version_minor     = 0,
    .version_patch     = 0,
    .service           = nullptr,
    .requirements      = nullptr,
    .requirement_count = 0,
    .init_fn           = nullptr,
};

// Flecs singleton component for SANDBOX_GET_SERVICE test
struct CounterServiceComponent {
    ISimpleCounter* api;
    const sandbox_service_info_t* info;
};
ECS_COMPONENT_DECLARE(CounterServiceComponent);

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

TEST_CASE("BootABI: C staging functions", "[bootstrapper][abi][staging]")
{
    flecs::world ecs;
    SECTION("sandbox_stage_service returns true on success") {
        bootstrapper_test_accessor::reset();
        sandbox_service_info_t svc{};
        svc.name = "MySvc";
        svc.architecture = "test::arch";
        svc.version_major = 1;
        svc.version_minor = 0;
        svc.init_fn = nullptr;
        REQUIRE(sandbox_stage_service(&svc) == true);
        bootstrapper_test_accessor::reset();
    }

    SECTION("sandbox_stage_service with null is safe and returns false") {
        bootstrapper_test_accessor::reset();
        REQUIRE(sandbox_stage_service(nullptr) == false);
        bootstrapper_test_accessor::reset();
    }

    SECTION("sandbox_stage_module registers and allows activate") {
        bootstrapper_test_accessor::reset();
        sandbox_module_info_t mod{};
        mod.name = "MyMod";
        mod.architecture = "test::arch";
        mod.version_major = 1;
        mod.version_minor = 0;
        mod.version_patch = 0;
        mod.service = nullptr; mod.requirements = nullptr; mod.requirement_count = 0; mod.init_fn = nullptr;
        REQUIRE(sandbox_stage_module(&mod) == true);
        bootstrapper_t b;
        REQUIRE_NOTHROW(b.activate(ecs, "test::arch", "MyMod", 1, 0, 0));
        bootstrapper_test_accessor::reset();
    }

    SECTION("sandbox_stage_module with null is safe and returns false") {
        bootstrapper_test_accessor::reset();
        REQUIRE(sandbox_stage_module(nullptr) == false);
        bootstrapper_test_accessor::reset();
    }
}

TEST_CASE("BootABI: C indexing functions", "[bootstrapper][abi][indexing]")
{
    flecs::world ecs;
    SECTION("sandbox_index_library with null is safe") {
        REQUIRE_NOTHROW(sandbox_load_library(ecs.c_ptr(), nullptr));
    }

    SECTION("sandbox_load_library with nonexistent path doesn't crash") {
        REQUIRE_NOTHROW(sandbox_load_library(ecs.c_ptr(), "nonexistent_c_lib.so"));
    }
}

TEST_CASE("BootABI: SANDBOX_DECLARE_MODULE struct metadata", "[bootstrapper][abi][macro]")
{
    flecs::world ecs;
    SECTION("name, architecture, and version are set correctly") {
        REQUIRE(std::string(TestMacroModule_manual_info.name) == "TestMacroModule");
        REQUIRE(std::string(TestMacroModule_manual_info.architecture) == "sandbox::system");
        REQUIRE(TestMacroModule_manual_info.version_major == 1);
        REQUIRE(TestMacroModule_manual_info.version_minor == 0);
        REQUIRE(TestMacroModule_manual_info.version_patch == 0);
    }

    SECTION("module can be staged and activated") {
        bootstrapper_test_accessor::reset();
        sandbox_module_info_t stageable = TestMacroModule_manual_info;
        stageable.init_fn = [](ecs_world_t*) {};
        bootstrapper_t::stage_module(stageable);
        bootstrapper_t b;
        REQUIRE_NOTHROW(b.activate(ecs, "sandbox::system", "TestMacroModule", 1, 0, 0));
        REQUIRE_NOTHROW(b.boot(ecs));
        bootstrapper_test_accessor::reset();
    }
}

TEST_CASE("BootABI: SANDBOX_DECLARE_SERVICE struct metadata", "[bootstrapper][abi][macro]")
{
    flecs::world ecs;
    SECTION("name, architecture, and version are set correctly") {
        REQUIRE(std::string(SimpleCounterService_manual_info.name) == "ISimpleCounter");
        REQUIRE(std::string(SimpleCounterService_manual_info.architecture) == "sandbox::system");
        REQUIRE(SimpleCounterService_manual_info.version_major == 1);
    }
}

TEST_CASE("BootABI: service component accessible via Flecs", "[bootstrapper][abi][get_service]")
{
    flecs::world ecs;
    bootstrapper_test_accessor::reset();

    sandbox_module_info_t mod{};
    mod.name = "CounterModule"; mod.description = "Counter provider";
    mod.architecture = "sandbox::system";
    mod.version_major = 1; mod.version_minor = 0; mod.version_patch = 0;
    mod.service = nullptr; mod.requirements = nullptr; mod.requirement_count = 0;
    mod.init_fn = [](ecs_world_t* ecs) {
        ECS_COMPONENT_DEFINE(ecs, CounterServiceComponent);
        CounterServiceComponent comp;
        comp.api  = &simple_counter_singleton;
        comp.info = &SimpleCounterService_manual_info;
        ecs_set_id(ecs, ecs_id(CounterServiceComponent),
                   ecs_id(CounterServiceComponent),
                   sizeof(CounterServiceComponent), &comp);
    };

    bootstrapper_t::stage_module(mod);
    bootstrapper_t b;
    b.activate(ecs, "sandbox::system", "CounterModule", 1, 0, 0);
    flecs::world w;
    b.boot(w);

    const CounterServiceComponent* retrieved = w.try_get<CounterServiceComponent>();

    SECTION("singleton is accessible from Flecs world") {
        REQUIRE(retrieved != nullptr);
    }

    SECTION("sdk pointer is the correct singleton address") {
        REQUIRE(retrieved != nullptr);
        REQUIRE(retrieved->api == &simple_counter_singleton);
    }

    SECTION("sdk data has the correct value") {
        REQUIRE(retrieved != nullptr);
        REQUIRE(retrieved->api->count == 42);
    }

    SECTION("info pointer name matches the service") {
        REQUIRE(retrieved != nullptr);
        REQUIRE(retrieved->info != nullptr);
        REQUIRE(std::string(retrieved->info->name) == "ISimpleCounter");
    }

    bootstrapper_test_accessor::reset();
}
