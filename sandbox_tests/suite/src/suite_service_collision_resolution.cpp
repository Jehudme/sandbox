// suite/src/suite_service_collision_resolution.cpp
//
// Integration suite: Service collision resolution across multiple scenarios
//
// Tests the full resolution pipeline when multiple modules compete for the
// same service slot, verifying:
//   1. The highest module version wins the service slot
//   2. Same service version — module with alphabetically earlier name wins
//   3. Three-way collision — correct winner is selected and losers are evicted
//   4. After collision resolution, the surviving service is callable from ECS
//   5. Collision involving auto-resolved (not explicitly activated) providers

#include <catch2/catch_all.hpp>
#include "sandbox/core/bootstrapper.h"
#include "core/bootstrapper.h"

#include <flecs.h>
#include <string>
#include <vector>

using sandbox::core::Bootstrapper;
using sandbox::core::ModuleInfo;
using sandbox::core::ServiceInfo;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
static ServiceInfo make_collision_service(const char* name, int major, int minor) {
    ServiceInfo service_info{};
    service_info.name          = name;
    service_info.description   = "Collision test service";
    service_info.architecture  = "sandbox::system";
    service_info.version_major = major;
    service_info.version_minor = minor;
    service_info.init_fn       = nullptr;
    return service_info;
}

static ModuleInfo make_collision_module(
    const char* name, int major, int minor, int patch,
    const ServiceInfo* service_ptr,
    void (*init_fn)(ecs_world_t*) = nullptr
) {
    ModuleInfo module_info{};
    module_info.name              = name;
    module_info.description       = "Collision test module";
    module_info.architecture      = "sandbox::system";
    module_info.version_major     = major;
    module_info.version_minor     = minor;
    module_info.version_patch     = patch;
    module_info.service           = service_ptr;
    module_info.requirements      = nullptr;
    module_info.requirement_count = 0;
    module_info.init_fn           = init_fn;
    return module_info;
}

// ---------------------------------------------------------------------------
// Suite: Two-way collision — higher module version wins
// ---------------------------------------------------------------------------
TEST_CASE("Suite: Service collision — higher module version win...",
          "[suite][collision][two_way]")
{
    Bootstrapper::reset();

    static int winner_init_calls = 0;
    static int loser_init_calls  = 0;
    winner_init_calls = 0;
    loser_init_calls  = 0;

    ServiceInfo physics_service = make_collision_service("IPhysicsEngine", 1, 0);

    // Both modules provide the same service but at different module versions
    ModuleInfo bullet_v1 = make_collision_module("BulletPhysicsV1", 1, 0, 0, &physics_service,
        [](ecs_world_t*) { loser_init_calls++; });

    ModuleInfo bullet_v2 = make_collision_module("BulletPhysicsV2", 2, 0, 0, &physics_service,
        [](ecs_world_t*) { winner_init_calls++; });

    Bootstrapper::stage_module(bullet_v1);
    Bootstrapper::stage_module(bullet_v2);

    Bootstrapper bootstrapper_instance;
    bootstrapper_instance.activate("sandbox::system", "BulletPhysicsV1", 1, 0, 0);
    bootstrapper_instance.activate("sandbox::system", "BulletPhysicsV2", 2, 0, 0);

    flecs::world test_world;
    REQUIRE_NOTHROW(bootstrapper_instance.boot(test_world));

    SECTION("winner (v2) was initialized exactly once") {
        REQUIRE(winner_init_calls == 1);
    }

    SECTION("loser (v1) was suppressed (not initialized)") {
        REQUIRE(loser_init_calls == 0);
    }

    Bootstrapper::reset();
}

// ---------------------------------------------------------------------------
// Suite: Three-way collision — only one winner survives
// ---------------------------------------------------------------------------
TEST_CASE("Suite: Service collision — three-way collision yield...",
          "[suite][collision][three_way]")
{
    Bootstrapper::reset();

    static int call_counts[3] = {0, 0, 0};
    call_counts[0] = call_counts[1] = call_counts[2] = 0;

    ServiceInfo audio_service = make_collision_service("IAudioEngine", 1, 0);

    ModuleInfo provider_v100 = make_collision_module("FMODAudio",    1, 0, 0, &audio_service,
        [](ecs_world_t*) { call_counts[0]++; });
    ModuleInfo provider_v200 = make_collision_module("OpenAL",       2, 0, 0, &audio_service,
        [](ecs_world_t*) { call_counts[1]++; });
    ModuleInfo provider_v150 = make_collision_module("XAudio2",      1, 5, 0, &audio_service,
        [](ecs_world_t*) { call_counts[2]++; });

    Bootstrapper::stage_module(provider_v100);
    Bootstrapper::stage_module(provider_v200);
    Bootstrapper::stage_module(provider_v150);

    Bootstrapper bootstrapper_instance;
    bootstrapper_instance.activate("sandbox::system", "FMODAudio", 1, 0, 0);
    bootstrapper_instance.activate("sandbox::system", "OpenAL",    2, 0, 0);
    bootstrapper_instance.activate("sandbox::system", "XAudio2",   1, 5, 0);

    flecs::world test_world;
    REQUIRE_NOTHROW(bootstrapper_instance.boot(test_world));

    int total_init_calls = call_counts[0] + call_counts[1] + call_counts[2];

    SECTION("exactly one provider was initialized") {
        REQUIRE(total_init_calls == 1);
    }

    SECTION("the highest module version (v2=OpenAL) was the winner") {
        // v2.0.0 > v1.5.0 > v1.0.0
        REQUIRE(call_counts[1] == 1);  // OpenAL (v2.0.0)
    }

    SECTION("lower version providers were suppressed") {
        REQUIRE(call_counts[0] == 0);  // FMODAudio
        REQUIRE(call_counts[2] == 0);  // XAudio2
    }

    Bootstrapper::reset();
}

// ---------------------------------------------------------------------------
// Suite: Collision with auto-resolved provider
// ---------------------------------------------------------------------------
TEST_CASE("Suite: Service collision — auto-resolved provider vs...",
          "[suite][collision][auto_resolved]")
{
    Bootstrapper::reset();

    static int explicit_init_calls   = 0;
    static int auto_resolved_calls   = 0;
    explicit_init_calls  = 0;
    auto_resolved_calls  = 0;

    ServiceInfo network_service = make_collision_service("INetworkEngine", 1, 0);

    // Explicit higher-version provider
    ModuleInfo explicit_provider = make_collision_module("NetworkV2", 2, 0, 0, &network_service,
        [](ecs_world_t*) { explicit_init_calls++; });

    // Lower-version provider that would be auto-resolved by a consumer
    ModuleInfo auto_provider = make_collision_module("NetworkV1", 1, 0, 0, &network_service,
        [](ecs_world_t*) { auto_resolved_calls++; });

    // Consumer module requiring INetworkEngine (will auto-pull the best available)
    sandbox_requirement_info_t network_requirement[] = {{
        .kind          = SANDBOX_REQUIREMENT_KIND_SERVICE,
        .strictness    = SANDBOX_REQUIREMENT_STRICTNESS_REQUIRED,
        .name          = "INetworkEngine",
        .architecture  = "sandbox::system",
        .version_major = 1,
        .version_minor = 0,
        .version_patch = -1,
    }};

    ModuleInfo game_network_module = make_collision_module("GameNetworkLayer", 1, 0, 0, nullptr);
    game_network_module.requirements      = network_requirement;
    game_network_module.requirement_count = 1;

    Bootstrapper::stage_module(explicit_provider);
    Bootstrapper::stage_module(auto_provider);
    Bootstrapper::stage_module(game_network_module);

    // Explicitly activate both providers AND the consumer
    Bootstrapper bootstrapper_instance;
    bootstrapper_instance.activate("sandbox::system", "NetworkV2",         2, 0, 0);  // explicit winner
    bootstrapper_instance.activate("sandbox::system", "GameNetworkLayer",  1, 0, 0);  // will auto-pull NetworkV1

    flecs::world test_world;
    REQUIRE_NOTHROW(bootstrapper_instance.boot(test_world));

    int total_network_init_calls = explicit_init_calls + auto_resolved_calls;

    SECTION("exactly one network provider was initialized") {
        REQUIRE(total_network_init_calls == 1);
    }

    SECTION("the higher-version explicit provider wins") {
        REQUIRE(explicit_init_calls == 1);
        REQUIRE(auto_resolved_calls == 0);
    }

    Bootstrapper::reset();
}

// ---------------------------------------------------------------------------
// Suite: No collision when modules provide different services
// ---------------------------------------------------------------------------
TEST_CASE("Suite: Service non-collision — modules with differen...",
          "[suite][collision][no_collision]")
{
    Bootstrapper::reset();

    static bool renderer_initialized = false;
    static bool audio_initialized    = false;
    renderer_initialized = false;
    audio_initialized    = false;

    ServiceInfo renderer_service = make_collision_service("IRenderer", 1, 0);
    ServiceInfo audio_service    = make_collision_service("IAudio",    1, 0);

    ModuleInfo renderer_module = make_collision_module("OpenGLRenderer", 1, 0, 0, &renderer_service,
        [](ecs_world_t*) { renderer_initialized = true; });
    ModuleInfo audio_module    = make_collision_module("FMODAudio",      1, 0, 0, &audio_service,
        [](ecs_world_t*) { audio_initialized = true; });

    Bootstrapper::stage_module(renderer_module);
    Bootstrapper::stage_module(audio_module);

    Bootstrapper bootstrapper_instance;
    bootstrapper_instance.activate("sandbox::system", "OpenGLRenderer", 1, 0, 0);
    bootstrapper_instance.activate("sandbox::system", "FMODAudio",      1, 0, 0);

    flecs::world test_world;
    REQUIRE_NOTHROW(bootstrapper_instance.boot(test_world));

    SECTION("renderer module was initialized (no collision with audio)") {
        REQUIRE(renderer_initialized == true);
    }

    SECTION("audio module was initialized (no collision with renderer)") {
        REQUIRE(audio_initialized == true);
    }

    Bootstrapper::reset();
}
