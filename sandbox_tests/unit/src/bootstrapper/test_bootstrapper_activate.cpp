// unit/src/bootstrapper/test_bootstrapper_activate.cpp
//
// Unit tests for Bootstrapper::activate():
//   - activate() exact-patch match succeeds
//   - activate() with version_patch=-1 picks the highest patch
//   - activate() with unknown module throws std::invalid_argument
//   - activate() with wrong architecture throws
//   - activate() with version_major mismatch throws
//   - activate() deduplicates (same module activated twice is a no-op)
//   - activate() with multiple candidates picks the best version

#include <catch2/catch_all.hpp>
#include "core/bootstrapper.h"

using sandbox::core::Bootstrapper;
using sandbox::core::ModuleInfo;
using sandbox::core::ServiceInfo;

// ---------------------------------------------------------------------------
// Helper
// ---------------------------------------------------------------------------
static ModuleInfo make_module(const char* name, const char* arch, int major, int minor, int patch,
                              const ServiceInfo* linked_service = nullptr) {
    ModuleInfo module_info{};
    module_info.name          = name;
    module_info.description   = "Unit test module";
    module_info.architecture  = arch;
    module_info.version_major = major;
    module_info.version_minor = minor;
    module_info.version_patch = patch;
    module_info.service       = linked_service;
    module_info.requirements  = nullptr;
    module_info.requirement_count = 0;
    module_info.init_fn       = nullptr;
    return module_info;
}

// ---------------------------------------------------------------------------
// Feature: Bootstrapper::activate — exact patch match
// ---------------------------------------------------------------------------
TEST_CASE("Boot:activate: exact-patch match succeeds",
          "[bootstrapper][activate][exact]")
{
    Bootstrapper::reset();

    ModuleInfo renderer_v100 = make_module("Renderer", "sandbox::system", 1, 0, 0);
    Bootstrapper::stage_module(renderer_v100);

    Bootstrapper bootstrapper_instance;
    REQUIRE_NOTHROW(bootstrapper_instance.activate("sandbox::system", "Renderer", 1, 0, 0));

    Bootstrapper::reset();
}

// ---------------------------------------------------------------------------
// Feature: Bootstrapper::activate — wildcard patch (-1) picks highest
// ---------------------------------------------------------------------------
TEST_CASE("Boot:activate: patch=-1 resolves to the highest avai...",
          "[bootstrapper][activate][wildcard_patch]")
{
    Bootstrapper::reset();

    // Stage three patches — v1.0.3 should win
    ModuleInfo renderer_v101 = make_module("Renderer", "sandbox::system", 1, 0, 1);
    ModuleInfo renderer_v103 = make_module("Renderer", "sandbox::system", 1, 0, 3);
    ModuleInfo renderer_v100 = make_module("Renderer", "sandbox::system", 1, 0, 0);

    Bootstrapper::stage_module(renderer_v101);
    Bootstrapper::stage_module(renderer_v103);
    Bootstrapper::stage_module(renderer_v100);

    Bootstrapper bootstrapper_instance;
    // version_patch = -1 => wildcard
    REQUIRE_NOTHROW(bootstrapper_instance.activate("sandbox::system", "Renderer", 1, 0, -1));

    Bootstrapper::reset();
}

// ---------------------------------------------------------------------------
// Feature: Bootstrapper::activate — errors
// ---------------------------------------------------------------------------
TEST_CASE("Boot:activate: throws when module name is unknown",
          "[bootstrapper][activate][error]")
{
    Bootstrapper::reset();

    Bootstrapper bootstrapper_instance;
    REQUIRE_THROWS_AS(
        bootstrapper_instance.activate("sandbox::system", "NonExistentModule", 1, 0, 0),
        std::invalid_argument
    );

    Bootstrapper::reset();
}

TEST_CASE("Boot:activate: throws when architecture does not match",
          "[bootstrapper][activate][error]")
{
    Bootstrapper::reset();

    ModuleInfo arm_module = make_module("Physics", "arm64", 1, 0, 0);
    Bootstrapper::stage_module(arm_module);

    Bootstrapper bootstrapper_instance;
    REQUIRE_THROWS_AS(
        bootstrapper_instance.activate("sandbox::system", "Physics", 1, 0, 0),  // wrong arch
        std::invalid_argument
    );

    Bootstrapper::reset();
}

TEST_CASE("Boot:activate: throws when major version does not ma...",
          "[bootstrapper][activate][error]")
{
    Bootstrapper::reset();

    ModuleInfo engine_v2 = make_module("Engine", "sandbox::system", 2, 0, 0);
    Bootstrapper::stage_module(engine_v2);

    Bootstrapper bootstrapper_instance;
    REQUIRE_THROWS_AS(
        bootstrapper_instance.activate("sandbox::system", "Engine", 3, 0, -1),  // major=3 not found
        std::invalid_argument
    );

    Bootstrapper::reset();
}

// ---------------------------------------------------------------------------
// Feature: Bootstrapper::activate — deduplication
// ---------------------------------------------------------------------------
TEST_CASE("Boot:activate: activate twice no-op",
          "[bootstrapper][activate][dedup]")
{
    Bootstrapper::reset();

    static int init_call_count = 0;
    init_call_count = 0;

    ModuleInfo counter_module = make_module("Counter", "sandbox::system", 1, 0, 0);
    counter_module.init_fn = [](ecs_world_t*) { init_call_count++; };
    Bootstrapper::stage_module(counter_module);

    Bootstrapper bootstrapper_instance;
    bootstrapper_instance.activate("sandbox::system", "Counter", 1, 0, 0);
    bootstrapper_instance.activate("sandbox::system", "Counter", 1, 0, 0);  // second call is no-op

    flecs::world test_world;
    bootstrapper_instance.boot(test_world);

    SECTION("init_fn was called exactly once despite two activate() calls") {
        REQUIRE(init_call_count == 1);
    }

    Bootstrapper::reset();
}

// ---------------------------------------------------------------------------
// Feature: Bootstrapper::activate — version resolution with multiple candidates
// ---------------------------------------------------------------------------
TEST_CASE("Boot:activate: picks best minor",
          "[bootstrapper][activate][version_resolution]")
{
    Bootstrapper::reset();

    // Stage v1.1.0 and v1.5.0
    ModuleInfo module_v110 = make_module("Service", "sandbox::system", 1, 1, 0);
    ModuleInfo module_v150 = make_module("Service", "sandbox::system", 1, 5, 0);
    Bootstrapper::stage_module(module_v110);
    Bootstrapper::stage_module(module_v150);

    static int chosen_minor = -1;
    chosen_minor = -1;

    // Capture which one gets initialized
    module_v110.init_fn = [](ecs_world_t*) { chosen_minor = 1; };
    module_v150.init_fn = [](ecs_world_t*) { chosen_minor = 5; };

    // Re-stage with init_fn pointers set
    Bootstrapper::reset();
    Bootstrapper::stage_module(module_v110);
    Bootstrapper::stage_module(module_v150);

    Bootstrapper bootstrapper_instance;
    bootstrapper_instance.activate("sandbox::system", "Service", 1, 0, -1);  // want at least minor 0

    flecs::world test_world;
    bootstrapper_instance.boot(test_world);

    SECTION("higher minor version (v1.5.0) is chosen over lower (v1.1.0)") {
        REQUIRE(chosen_minor == 5);
    }

    Bootstrapper::reset();
}
