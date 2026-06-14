// unit/src/bootstrapper/test_bootstrapper_boot.cpp
//
// Unit tests for Bootstrapper::boot():
//   - Modules with no requirements boot in any valid order
//   - Required service dependency is auto-resolved from the global registry
//   - Required module dependency is auto-resolved
//   - Expected (soft) dependency is included when available
//   - Expected dependency is silently skipped when unavailable
//   - Service collision evicts the loser (lower version)
//   - Cyclic dependency between modules throws std::runtime_error
//   - init_fn call order respects dependencies (provider before consumer)
//   - Major version collision between two required services throws

#include <catch2/catch_all.hpp>
#include "core/bootstrapper.h"

#include <vector>
#include <string>

using sandbox::core::Bootstrapper;
using sandbox::core::ModuleInfo;
using sandbox::core::ServiceInfo;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
static ServiceInfo make_service(const char* name, int major, int minor, void (*init_fn)(ecs_world_t*) = nullptr) {
    ServiceInfo service_info{};
    service_info.name          = name;
    service_info.description   = "Boot test service";
    service_info.architecture  = "x86_64";
    service_info.version_major = major;
    service_info.version_minor = minor;
    service_info.init_fn       = init_fn;
    return service_info;
}

static ModuleInfo make_standalone_module(const char* name, int major, int minor, int patch,
                                         void (*init_fn)(ecs_world_t*) = nullptr) {
    ModuleInfo module_info{};
    module_info.name              = name;
    module_info.description       = "Boot test standalone module";
    module_info.architecture      = "x86_64";
    module_info.version_major     = major;
    module_info.version_minor     = minor;
    module_info.version_patch     = patch;
    module_info.service           = nullptr;
    module_info.requirements      = nullptr;
    module_info.requirement_count = 0;
    module_info.init_fn           = init_fn;
    return module_info;
}

// ---------------------------------------------------------------------------
// Feature: Bootstrapper::boot — modules with no requirements
// ---------------------------------------------------------------------------
TEST_CASE("Bootstrapper::boot: modules without requirements all get initialized",
          "[bootstrapper][boot][no_deps]")
{
    Bootstrapper::reset();

    static int module_a_calls = 0;
    static int module_b_calls = 0;
    module_a_calls = 0;
    module_b_calls = 0;

    ModuleInfo module_alpha = make_standalone_module("Alpha", 1, 0, 0, [](ecs_world_t*) { module_a_calls++; });
    ModuleInfo module_beta  = make_standalone_module("Beta",  1, 0, 0, [](ecs_world_t*) { module_b_calls++; });

    Bootstrapper::stage_module(module_alpha);
    Bootstrapper::stage_module(module_beta);

    Bootstrapper bootstrapper_instance;
    bootstrapper_instance.activate("x86_64", "Alpha", 1, 0, 0);
    bootstrapper_instance.activate("x86_64", "Beta",  1, 0, 0);

    flecs::world test_world;
    bootstrapper_instance.boot(test_world);

    SECTION("Alpha was initialized exactly once") {
        REQUIRE(module_a_calls == 1);
    }

    SECTION("Beta was initialized exactly once") {
        REQUIRE(module_b_calls == 1);
    }

    Bootstrapper::reset();
}

// ---------------------------------------------------------------------------
// Feature: Bootstrapper::boot — required service dependency auto-resolution
// ---------------------------------------------------------------------------
TEST_CASE("Bootstrapper::boot: required service dependency is auto-pulled in",
          "[bootstrapper][boot][required_service]")
{
    Bootstrapper::reset();

    static std::vector<std::string> boot_order;
    boot_order.clear();

    // Provider module: provides ILogger service
    ServiceInfo logger_service = make_service("ILogger", 1, 0,
        [](ecs_world_t*) { boot_order.push_back("ILogger"); });

    ModuleInfo logger_provider = make_standalone_module("LoggerImpl", 1, 0, 0,
        [](ecs_world_t*) { boot_order.push_back("LoggerImpl"); });
    logger_provider.service = &logger_service;

    // Consumer module: requires ILogger service
    sandbox_requirement_info_t consumer_requirements[1];
    consumer_requirements[0].kind         = SANDBOX_REQUIREMENT_KIND_SERVICE;
    consumer_requirements[0].strictness   = SANDBOX_REQUIREMENT_STRICTNESS_REQUIRED;
    consumer_requirements[0].name         = "ILogger";
    consumer_requirements[0].architecture = "x86_64";
    consumer_requirements[0].version_major = 1;
    consumer_requirements[0].version_minor = 0;
    consumer_requirements[0].version_patch = -1;

    ModuleInfo consumer_module = make_standalone_module("ConsumerModule", 1, 0, 0,
        [](ecs_world_t*) { boot_order.push_back("ConsumerModule"); });
    consumer_module.requirements      = consumer_requirements;
    consumer_module.requirement_count = 1;

    Bootstrapper::stage_module(logger_provider);
    Bootstrapper::stage_module(consumer_module);

    // Only activate the consumer — provider should be auto-pulled
    Bootstrapper bootstrapper_instance;
    bootstrapper_instance.activate("x86_64", "ConsumerModule", 1, 0, 0);

    flecs::world test_world;
    REQUIRE_NOTHROW(bootstrapper_instance.boot(test_world));

    SECTION("provider module was initialized (auto-resolved)") {
        REQUIRE(std::find(boot_order.begin(), boot_order.end(), "LoggerImpl") != boot_order.end());
    }

    SECTION("consumer module was initialized") {
        REQUIRE(std::find(boot_order.begin(), boot_order.end(), "ConsumerModule") != boot_order.end());
    }

    SECTION("provider boots before consumer (dependency ordering)") {
        auto provider_position = std::find(boot_order.begin(), boot_order.end(), "LoggerImpl");
        auto consumer_position = std::find(boot_order.begin(), boot_order.end(), "ConsumerModule");
        REQUIRE(provider_position < consumer_position);
    }

    Bootstrapper::reset();
}

// ---------------------------------------------------------------------------
// Feature: Bootstrapper::boot — required module dependency auto-resolution
// ---------------------------------------------------------------------------
TEST_CASE("Bootstrapper::boot: required module dependency is auto-pulled in",
          "[bootstrapper][boot][required_module]")
{
    Bootstrapper::reset();

    static bool dependency_initialized  = false;
    static bool consumer_initialized    = false;
    dependency_initialized = false;
    consumer_initialized   = false;

    ModuleInfo dependency_module = make_standalone_module("MathLib", 1, 0, 0,
        [](ecs_world_t*) { dependency_initialized = true; });

    sandbox_requirement_info_t math_requirement[1];
    math_requirement[0].kind          = SANDBOX_REQUIREMENT_KIND_MODULE;
    math_requirement[0].strictness    = SANDBOX_REQUIREMENT_STRICTNESS_REQUIRED;
    math_requirement[0].name          = "MathLib";
    math_requirement[0].architecture  = "x86_64";
    math_requirement[0].version_major = 1;
    math_requirement[0].version_minor = 0;
    math_requirement[0].version_patch = -1;

    ModuleInfo physics_module = make_standalone_module("PhysicsEngine", 1, 0, 0,
        [](ecs_world_t*) { consumer_initialized = true; });
    physics_module.requirements      = math_requirement;
    physics_module.requirement_count = 1;

    Bootstrapper::stage_module(dependency_module);
    Bootstrapper::stage_module(physics_module);

    Bootstrapper bootstrapper_instance;
    bootstrapper_instance.activate("x86_64", "PhysicsEngine", 1, 0, 0);

    flecs::world test_world;
    REQUIRE_NOTHROW(bootstrapper_instance.boot(test_world));

    SECTION("required dependency (MathLib) was auto-initialized") {
        REQUIRE(dependency_initialized == true);
    }

    SECTION("consumer (PhysicsEngine) was initialized") {
        REQUIRE(consumer_initialized == true);
    }

    Bootstrapper::reset();
}

// ---------------------------------------------------------------------------
// Feature: Bootstrapper::boot — required service missing throws
// ---------------------------------------------------------------------------
TEST_CASE("Bootstrapper::boot: missing required service throws std::runtime_error",
          "[bootstrapper][boot][error]")
{
    Bootstrapper::reset();

    sandbox_requirement_info_t missing_service_req[1];
    missing_service_req[0].kind          = SANDBOX_REQUIREMENT_KIND_SERVICE;
    missing_service_req[0].strictness    = SANDBOX_REQUIREMENT_STRICTNESS_REQUIRED;
    missing_service_req[0].name          = "INonExistentService";
    missing_service_req[0].architecture  = "x86_64";
    missing_service_req[0].version_major = 1;
    missing_service_req[0].version_minor = 0;
    missing_service_req[0].version_patch = -1;

    ModuleInfo needy_module = make_standalone_module("NeedyModule", 1, 0, 0);
    needy_module.requirements      = missing_service_req;
    needy_module.requirement_count = 1;

    Bootstrapper::stage_module(needy_module);

    Bootstrapper bootstrapper_instance;
    bootstrapper_instance.activate("x86_64", "NeedyModule", 1, 0, 0);

    flecs::world test_world;
    REQUIRE_THROWS_AS(bootstrapper_instance.boot(test_world), std::runtime_error);

    Bootstrapper::reset();
}

// ---------------------------------------------------------------------------
// Feature: Bootstrapper::boot — expected (soft) dependency is optional
// ---------------------------------------------------------------------------
TEST_CASE("Bootstrapper::boot: expected dependency is silently skipped when unavailable",
          "[bootstrapper][boot][expected_optional]")
{
    Bootstrapper::reset();

    static bool consumer_was_initialized = false;
    consumer_was_initialized = false;

    sandbox_requirement_info_t optional_req[1];
    optional_req[0].kind          = SANDBOX_REQUIREMENT_KIND_MODULE;
    optional_req[0].strictness    = SANDBOX_REQUIREMENT_STRICTNESS_EXPECTED;
    optional_req[0].name          = "OptionalModule";
    optional_req[0].architecture  = "x86_64";
    optional_req[0].version_major = 1;
    optional_req[0].version_minor = 0;
    optional_req[0].version_patch = -1;

    ModuleInfo flexible_module = make_standalone_module("FlexibleModule", 1, 0, 0,
        [](ecs_world_t*) { consumer_was_initialized = true; });
    flexible_module.requirements      = optional_req;
    flexible_module.requirement_count = 1;

    Bootstrapper::stage_module(flexible_module);  // optional module NOT staged

    Bootstrapper bootstrapper_instance;
    bootstrapper_instance.activate("x86_64", "FlexibleModule", 1, 0, 0);

    flecs::world test_world;
    REQUIRE_NOTHROW(bootstrapper_instance.boot(test_world));

    SECTION("consumer module still initialized even though expected dep is absent") {
        REQUIRE(consumer_was_initialized == true);
    }

    Bootstrapper::reset();
}

// ---------------------------------------------------------------------------
// Feature: Bootstrapper::boot — service collision eviction
// ---------------------------------------------------------------------------
TEST_CASE("Bootstrapper::boot: service collision evicts lower-version provider",
          "[bootstrapper][boot][collision]")
{
    Bootstrapper::reset();

    static int winner_init_calls = 0;
    static int loser_init_calls  = 0;
    winner_init_calls = 0;
    loser_init_calls  = 0;

    ServiceInfo renderer_service_v1 = make_service("IRenderer", 1, 0);
    ServiceInfo renderer_service_v2 = make_service("IRenderer", 2, 0);

    ModuleInfo low_version_provider  = make_standalone_module("OpenGLv1", 1, 0, 0,
        [](ecs_world_t*) { loser_init_calls++; });
    low_version_provider.service = &renderer_service_v1;

    ModuleInfo high_version_provider = make_standalone_module("VulkanRenderer", 2, 0, 0,
        [](ecs_world_t*) { winner_init_calls++; });
    high_version_provider.service = &renderer_service_v2;

    Bootstrapper::stage_module(low_version_provider);
    Bootstrapper::stage_module(high_version_provider);

    Bootstrapper bootstrapper_instance;
    bootstrapper_instance.activate("x86_64", "OpenGLv1",      1, 0, 0);
    bootstrapper_instance.activate("x86_64", "VulkanRenderer", 2, 0, 0);

    flecs::world test_world;
    REQUIRE_NOTHROW(bootstrapper_instance.boot(test_world));

    SECTION("winner (higher version) was initialized") {
        REQUIRE(winner_init_calls == 1);
    }

    SECTION("loser (lower version) was suppressed") {
        REQUIRE(loser_init_calls == 0);
    }

    Bootstrapper::reset();
}

// ---------------------------------------------------------------------------
// Feature: Bootstrapper::boot — cyclic dependency detection
// ---------------------------------------------------------------------------
TEST_CASE("Bootstrapper::boot: cyclic module dependency throws std::runtime_error",
          "[bootstrapper][boot][cyclic]")
{
    Bootstrapper::reset();

    // Module A requires Module B and Module B requires Module A
    sandbox_requirement_info_t req_b[1];
    req_b[0].kind          = SANDBOX_REQUIREMENT_KIND_MODULE;
    req_b[0].strictness    = SANDBOX_REQUIREMENT_STRICTNESS_REQUIRED;
    req_b[0].name          = "ModuleB";
    req_b[0].architecture  = "x86_64";
    req_b[0].version_major = 0;
    req_b[0].version_minor = 0;
    req_b[0].version_patch = -1;

    sandbox_requirement_info_t req_a[1];
    req_a[0].kind          = SANDBOX_REQUIREMENT_KIND_MODULE;
    req_a[0].strictness    = SANDBOX_REQUIREMENT_STRICTNESS_REQUIRED;
    req_a[0].name          = "ModuleA";
    req_a[0].architecture  = "x86_64";
    req_a[0].version_major = 0;
    req_a[0].version_minor = 0;
    req_a[0].version_patch = -1;

    ModuleInfo module_a = make_standalone_module("ModuleA", 1, 0, 0);
    module_a.requirements      = req_b;
    module_a.requirement_count = 1;

    ModuleInfo module_b = make_standalone_module("ModuleB", 1, 0, 0);
    module_b.requirements      = req_a;
    module_b.requirement_count = 1;

    Bootstrapper::stage_module(module_a);
    Bootstrapper::stage_module(module_b);

    Bootstrapper bootstrapper_instance;
    bootstrapper_instance.activate("x86_64", "ModuleA", 1, 0, 0);
    bootstrapper_instance.activate("x86_64", "ModuleB", 1, 0, 0);

    flecs::world test_world;
    REQUIRE_THROWS_AS(bootstrapper_instance.boot(test_world), std::runtime_error);

    Bootstrapper::reset();
}

// ---------------------------------------------------------------------------
// Feature: Bootstrapper::boot — major version service collision
// ---------------------------------------------------------------------------
TEST_CASE("Bootstrapper::boot: two required consumers disagreeing on service major version throws",
          "[bootstrapper][boot][version_collision]")
{
    Bootstrapper::reset();

    ServiceInfo logger_v1_service = make_service("ILogger", 1, 0);
    ServiceInfo logger_v2_service = make_service("ILogger", 2, 0);

    ModuleInfo logger_v1_provider = make_standalone_module("LoggerV1", 1, 0, 0);
    logger_v1_provider.service = &logger_v1_service;

    ModuleInfo logger_v2_provider = make_standalone_module("LoggerV2", 2, 0, 0);
    logger_v2_provider.service = &logger_v2_service;

    // Consumer A: requires ILogger v1
    sandbox_requirement_info_t req_logger_v1[1];
    req_logger_v1[0].kind          = SANDBOX_REQUIREMENT_KIND_SERVICE;
    req_logger_v1[0].strictness    = SANDBOX_REQUIREMENT_STRICTNESS_REQUIRED;
    req_logger_v1[0].name          = "ILogger";
    req_logger_v1[0].architecture  = "x86_64";
    req_logger_v1[0].version_major = 1;
    req_logger_v1[0].version_minor = 0;
    req_logger_v1[0].version_patch = -1;

    // Consumer B: requires ILogger v2
    sandbox_requirement_info_t req_logger_v2[1];
    req_logger_v2[0].kind          = SANDBOX_REQUIREMENT_KIND_SERVICE;
    req_logger_v2[0].strictness    = SANDBOX_REQUIREMENT_STRICTNESS_REQUIRED;
    req_logger_v2[0].name          = "ILogger";
    req_logger_v2[0].architecture  = "x86_64";
    req_logger_v2[0].version_major = 2;
    req_logger_v2[0].version_minor = 0;
    req_logger_v2[0].version_patch = -1;

    ModuleInfo consumer_a = make_standalone_module("ConsumerA", 1, 0, 0);
    consumer_a.requirements      = req_logger_v1;
    consumer_a.requirement_count = 1;

    ModuleInfo consumer_b = make_standalone_module("ConsumerB", 1, 0, 0);
    consumer_b.requirements      = req_logger_v2;
    consumer_b.requirement_count = 1;

    Bootstrapper::stage_module(logger_v1_provider);
    Bootstrapper::stage_module(logger_v2_provider);
    Bootstrapper::stage_module(consumer_a);
    Bootstrapper::stage_module(consumer_b);

    Bootstrapper bootstrapper_instance;
    bootstrapper_instance.activate("x86_64", "ConsumerA", 1, 0, 0);
    bootstrapper_instance.activate("x86_64", "ConsumerB", 1, 0, 0);

    flecs::world test_world;
    REQUIRE_THROWS_AS(bootstrapper_instance.boot(test_world), std::runtime_error);

    Bootstrapper::reset();
}
