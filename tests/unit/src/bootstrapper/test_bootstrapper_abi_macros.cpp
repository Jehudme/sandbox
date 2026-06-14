// unit/src/bootstrapper/test_bootstrapper_abi_macros.cpp
// Tests for the bootstrapper C ABI functions and macro-generated structs.

#include <catch2/catch_all.hpp>
#include "sandbox/core/bootstrapper.h"
#include "core/bootstrapper.h"
#include "sandbox/core/platform.h"

#include <flecs.h>

using sandbox::core::Bootstrapper;

// ---------------------------------------------------------------------------
// Minimal Flecs modules for import tests
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
    SECTION("sandbox_stage_service does not crash") {
        Bootstrapper::reset();
        sandbox_service_info_t svc{};
        svc.name = "IAudio"; svc.description = "ABI test"; svc.architecture = "sandbox::system";
        svc.version_major = 1; svc.version_minor = 0; svc.init_fn = nullptr;
        REQUIRE_NOTHROW(sandbox_stage_service(&svc));
        Bootstrapper::reset();
    }

    SECTION("sandbox_stage_service with null is safe") {
        Bootstrapper::reset();
        REQUIRE_NOTHROW(sandbox_stage_service(nullptr));
        Bootstrapper::reset();
    }

    SECTION("sandbox_stage_module registers and allows activate") {
        Bootstrapper::reset();
        sandbox_module_info_t mod{};
        mod.name = "VideoModule"; mod.description = "ABI test"; mod.architecture = "sandbox::system";
        mod.version_major = 1; mod.version_minor = 0; mod.version_patch = 0;
        mod.service = nullptr; mod.requirements = nullptr; mod.requirement_count = 0; mod.init_fn = nullptr;
        REQUIRE_NOTHROW(sandbox_stage_module(&mod));
        Bootstrapper b;
        REQUIRE_NOTHROW(b.activate("sandbox::system", "VideoModule", 1, 0, 0));
        Bootstrapper::reset();
    }

    SECTION("sandbox_stage_module with null is safe") {
        Bootstrapper::reset();
        REQUIRE_NOTHROW(sandbox_stage_module(nullptr));
        Bootstrapper::reset();
    }
}

TEST_CASE("BootABI: SANDBOX_DECLARE_MODULE struct metadata", "[bootstrapper][abi][macro]")
{
    SECTION("name, architecture, and version are set correctly") {
        REQUIRE(std::string(TestMacroModule_manual_info.name) == "TestMacroModule");
        REQUIRE(std::string(TestMacroModule_manual_info.architecture) == "sandbox::system");
        REQUIRE(TestMacroModule_manual_info.version_major == 1);
        REQUIRE(TestMacroModule_manual_info.version_minor == 0);
        REQUIRE(TestMacroModule_manual_info.version_patch == 0);
    }

    SECTION("module can be staged and activated") {
        Bootstrapper::reset();
        sandbox_module_info_t stageable = TestMacroModule_manual_info;
        stageable.init_fn = [](ecs_world_t*) {};
        Bootstrapper::stage_module(stageable);
        Bootstrapper b;
        REQUIRE_NOTHROW(b.activate("sandbox::system", "TestMacroModule", 1, 0, 0));
        flecs::world w;
        REQUIRE_NOTHROW(b.boot(w));
        Bootstrapper::reset();
    }
}

TEST_CASE("BootABI: SANDBOX_DECLARE_SERVICE struct metadata", "[bootstrapper][abi][macro]")
{
    SECTION("name, architecture, and version are set correctly") {
        REQUIRE(std::string(SimpleCounterService_manual_info.name) == "ISimpleCounter");
        REQUIRE(std::string(SimpleCounterService_manual_info.architecture) == "sandbox::system");
        REQUIRE(SimpleCounterService_manual_info.version_major == 1);
    }
}

TEST_CASE("BootABI: service component accessible via Flecs", "[bootstrapper][abi][get_service]")
{
    Bootstrapper::reset();

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

    Bootstrapper::stage_module(mod);
    Bootstrapper b;
    b.activate("sandbox::system", "CounterModule", 1, 0, 0);
    flecs::world w;
    b.boot(w);

    const CounterServiceComponent* retrieved = w.try_get<CounterServiceComponent>();

    SECTION("singleton is accessible from Flecs world") {
        REQUIRE(retrieved != nullptr);
    }

    SECTION("api pointer is the correct singleton address") {
        REQUIRE(retrieved != nullptr);
        REQUIRE(retrieved->api == &simple_counter_singleton);
    }

    SECTION("api data has the correct value") {
        REQUIRE(retrieved != nullptr);
        REQUIRE(retrieved->api->count == 42);
    }

    SECTION("info pointer name matches the service") {
        REQUIRE(retrieved != nullptr);
        REQUIRE(retrieved->info != nullptr);
        REQUIRE(std::string(retrieved->info->name) == "ISimpleCounter");
    }

    Bootstrapper::reset();
}
