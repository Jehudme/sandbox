// unit/src/bootstrapper/test_bootstrapper_staging.cpp
//
// Unit tests for Bootstrapper staging (stage_service / stage_module / reset):
//   - stage_service registers a unique service
//   - stage_service deduplicates identical registrations
//   - stage_module registers a unique module
//   - stage_module deduplicates identical registrations
//   - reset() clears all staged services and modules

#include <catch2/catch_all.hpp>
#include "core/bootstrapper.h"  // internal C++ class

using sandbox::core::Bootstrapper;
using sandbox::core::ServiceInfo;
using sandbox::core::ModuleInfo;

// ---------------------------------------------------------------------------
// Helpers — minimal valid info structs for staging
// ---------------------------------------------------------------------------
static ServiceInfo make_service(const char* name, int major, int minor) {
    ServiceInfo service_info{};
    service_info.name         = name;
    service_info.description  = "Test service";
    service_info.architecture = "test_arch";
    service_info.version_major = major;
    service_info.version_minor = minor;
    service_info.init_fn      = nullptr;
    return service_info;
}

static ModuleInfo make_module(const char* name, int major, int minor, int patch,
                              const ServiceInfo* linked_service = nullptr) {
    ModuleInfo module_info{};
    module_info.name         = name;
    module_info.description  = "Test module";
    module_info.architecture = "test_arch";
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
// Feature: Bootstrapper — stage_service
// ---------------------------------------------------------------------------
TEST_CASE("Boot:stage_service: registers a new service",
          "[bootstrapper][staging][service]")
{
    Bootstrapper::reset();

    ServiceInfo renderer_service = make_service("IRenderer", 1, 0);
    Bootstrapper::stage_service(renderer_service);

    // The only observable side-effect is that activate() can find the service
    // (tested indirectly via stage_module + activate, but we verify no throw here)
    ModuleInfo renderer_module = make_module("OpenGLRenderer", 1, 0, 0, &renderer_service);
    Bootstrapper::stage_module(renderer_module);

    SECTION("activate succeeds when the module was staged") {
        Bootstrapper bootstrapper_instance;
        REQUIRE_NOTHROW(bootstrapper_instance.activate("test_arch", "OpenGLRenderer", 1, 0, 0));
    }

    Bootstrapper::reset();
}

TEST_CASE("Boot:stage_service: dedup duplicates",
          "[bootstrapper][staging][service]")
{
    Bootstrapper::reset();

    ServiceInfo physics_service = make_service("IPhysics", 2, 0);

    // Register twice — should not create two entries
    Bootstrapper::stage_service(physics_service);
    Bootstrapper::stage_service(physics_service);

    ModuleInfo physics_module_a = make_module("BulletPhysics", 2, 0, 0, &physics_service);
    ModuleInfo physics_module_b = make_module("BulletPhysicsAlt", 2, 0, 0, &physics_service);
    Bootstrapper::stage_module(physics_module_a);
    Bootstrapper::stage_module(physics_module_b);

    // Activate one — it should resolve fine without extra collision
    Bootstrapper bootstrapper_instance;
    REQUIRE_NOTHROW(bootstrapper_instance.activate("test_arch", "BulletPhysics", 2, 0, 0));

    Bootstrapper::reset();
}

// ---------------------------------------------------------------------------
// Feature: Bootstrapper — stage_module
// ---------------------------------------------------------------------------
TEST_CASE("Boot:stage_module: registers a new module",
          "[bootstrapper][staging][module]")
{
    Bootstrapper::reset();

    ModuleInfo audio_module = make_module("AudioEngine", 1, 0, 0);
    Bootstrapper::stage_module(audio_module);

    Bootstrapper bootstrapper_instance;
    REQUIRE_NOTHROW(bootstrapper_instance.activate("test_arch", "AudioEngine", 1, 0, 0));

    Bootstrapper::reset();
}

TEST_CASE("Boot:stage_module: dedup duplicates",
          "[bootstrapper][staging][module]")
{
    Bootstrapper::reset();

    ModuleInfo network_module = make_module("NetworkModule", 1, 2, 3);

    // Register twice — should not double-add
    Bootstrapper::stage_module(network_module);
    Bootstrapper::stage_module(network_module);

    // activate() should work normally — no assertion, no duplicate entries causing issues
    Bootstrapper bootstrapper_instance;
    REQUIRE_NOTHROW(bootstrapper_instance.activate("test_arch", "NetworkModule", 1, 2, 3));

    Bootstrapper::reset();
}

// ---------------------------------------------------------------------------
// Feature: Bootstrapper — reset()
// ---------------------------------------------------------------------------
TEST_CASE("Boot:reset: clears all",
          "[bootstrapper][staging][reset]")
{
    // Stage some content
    ServiceInfo dummy_service = make_service("IDummy", 1, 0);
    ModuleInfo  dummy_module  = make_module("DummyModule", 1, 0, 0, &dummy_service);

    Bootstrapper::stage_service(dummy_service);
    Bootstrapper::stage_module(dummy_module);

    // Reset clears everything
    Bootstrapper::reset();

    // After reset, activating the same module must fail (nothing registered)
    Bootstrapper bootstrapper_instance;
    REQUIRE_THROWS_AS(
        bootstrapper_instance.activate("test_arch", "DummyModule", 1, 0, 0),
        std::invalid_argument
    );
}

TEST_CASE("Boot:reset: safe multi calls",
          "[bootstrapper][staging][reset]")
{
    Bootstrapper::reset();
    REQUIRE_NOTHROW(Bootstrapper::reset());
    REQUIRE_NOTHROW(Bootstrapper::reset());
}
