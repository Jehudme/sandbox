// suite/src/suite_service_collision_resolution.cpp
// Integration: Service slot collision — winner selection across scenarios.

#include <catch2/catch_all.hpp>
#include "../../../sandbox/include/sandbox/abi/bootstrapper.h"
#include "core/bootstrapper.h"

#include <flecs.h>
#include <string>
#include <vector>

using sandbox::core::bootstrapper_t;
using sandbox::core::module_info_t;
using sandbox::core::service_info_t;

static service_info_t make_svc(const char* name, int major, int minor) {
    service_info_t s{};
    s.name = name; s.description = "Collision svc"; s.architecture = "sandbox::system";
    s.version_major = major; s.version_minor = minor; s.init_fn = nullptr;
    return s;
}

static module_info_t make_mod(const char* name, int major, int minor, int patch,
                            const service_info_t* svc, void (*fn)(ecs_world_t*) = nullptr) {
    module_info_t m{};
    m.name = name; m.description = "Collision mod"; m.architecture = "sandbox::system";
    m.version_major = major; m.version_minor = minor; m.version_patch = patch;
    m.service = svc; m.requirements = nullptr; m.requirement_count = 0; m.init_fn = fn;
    return m;
}

// ---------------------------------------------------------------------------
// Suite: Service collision scenarios
// ---------------------------------------------------------------------------
TEST_CASE("Suite: Service collision — winner selection", "[suite][collision]")
{
    SECTION("two-way: higher module version wins") {
        bootstrapper_t::reset();
        static int winner = 0, loser = 0;
        winner = loser = 0;
        service_info_t svc = make_svc("IPhysicsEngine", 1, 0);
        auto v1 = make_mod("BulletPhysicsV1", 1, 0, 0, &svc, [](ecs_world_t*) { loser++; });
        auto v2 = make_mod("BulletPhysicsV2", 2, 0, 0, &svc, [](ecs_world_t*) { winner++; });
        bootstrapper_t::stage_module(v1);
        bootstrapper_t::stage_module(v2);
        bootstrapper_t b;
        b.activate("sandbox::system", "BulletPhysicsV1", 1, 0, 0);
        b.activate("sandbox::system", "BulletPhysicsV2", 2, 0, 0);
        flecs::world w;
        REQUIRE_NOTHROW(b.boot(w));
        REQUIRE(winner == 1);
        REQUIRE(loser  == 0);
        bootstrapper_t::reset();
    }

    SECTION("three-way: highest version among three wins") {
        bootstrapper_t::reset();
        static int calls[3] = {0, 0, 0};
        calls[0] = calls[1] = calls[2] = 0;
        service_info_t svc = make_svc("IAudioEngine", 1, 0);
        auto fmod   = make_mod("FMODAudio", 1, 0, 0, &svc, [](ecs_world_t*) { calls[0]++; });
        auto openal = make_mod("OpenAL",    2, 0, 0, &svc, [](ecs_world_t*) { calls[1]++; });
        auto xaudio = make_mod("XAudio2",   1, 5, 0, &svc, [](ecs_world_t*) { calls[2]++; });
        bootstrapper_t::stage_module(fmod);
        bootstrapper_t::stage_module(openal);
        bootstrapper_t::stage_module(xaudio);
        bootstrapper_t b;
        b.activate("sandbox::system", "FMODAudio", 1, 0, 0);
        b.activate("sandbox::system", "OpenAL",    2, 0, 0);
        b.activate("sandbox::system", "XAudio2",   1, 5, 0);
        flecs::world w;
        REQUIRE_NOTHROW(b.boot(w));
        REQUIRE(calls[0] + calls[1] + calls[2] == 1);  // exactly one winner
        REQUIRE(calls[1] == 1);                          // OpenAL v2 wins
        REQUIRE(calls[0] == 0);
        REQUIRE(calls[2] == 0);
        bootstrapper_t::reset();
    }

    SECTION("auto-resolved provider loses to explicit higher-version provider") {
        bootstrapper_t::reset();
        static int explicit_calls = 0, auto_calls = 0;
        explicit_calls = auto_calls = 0;
        service_info_t svc = make_svc("INetworkEngine", 1, 0);
        auto high = make_mod("NetworkV2", 2, 0, 0, &svc, [](ecs_world_t*) { explicit_calls++; });
        auto low  = make_mod("NetworkV1", 1, 0, 0, &svc, [](ecs_world_t*) { auto_calls++; });

        sandbox_requirement_info_t req[] = {{
            SANDBOX_REQUIREMENT_KIND_SERVICE, SANDBOX_REQUIREMENT_STRICTNESS_REQUIRED,
            "INetworkEngine", "sandbox::system", 1, 0, -1
        }};
        auto consumer = make_mod("GameNetworkLayer", 1, 0, 0, nullptr);
        consumer.requirements = req;
        consumer.requirement_count = 1;

        bootstrapper_t::stage_module(high);
        bootstrapper_t::stage_module(low);
        bootstrapper_t::stage_module(consumer);
        bootstrapper_t b;
        b.activate("sandbox::system", "NetworkV2",        2, 0, 0);
        b.activate("sandbox::system", "GameNetworkLayer", 1, 0, 0);
        flecs::world w;
        REQUIRE_NOTHROW(b.boot(w));
        REQUIRE(explicit_calls + auto_calls == 1);
        REQUIRE(explicit_calls == 1);
        REQUIRE(auto_calls     == 0);
        bootstrapper_t::reset();
    }
}

TEST_CASE("Suite: No collision between different services", "[suite][collision]")
{
    bootstrapper_t::reset();
    static bool renderer_init = false, audio_init = false;
    renderer_init = audio_init = false;

    service_info_t rend_svc  = make_svc("IRenderer", 1, 0);
    service_info_t audio_svc = make_svc("IAudio",    1, 0);
    auto rend = make_mod("OpenGLRenderer", 1, 0, 0, &rend_svc,  [](ecs_world_t*) { renderer_init = true; });
    auto aud  = make_mod("FMODAudio",      1, 0, 0, &audio_svc, [](ecs_world_t*) { audio_init    = true; });

    bootstrapper_t::stage_module(rend);
    bootstrapper_t::stage_module(aud);
    bootstrapper_t b;
    b.activate("sandbox::system", "OpenGLRenderer", 1, 0, 0);
    b.activate("sandbox::system", "FMODAudio",      1, 0, 0);
    flecs::world w;
    REQUIRE_NOTHROW(b.boot(w));

    SECTION("renderer initialized despite audio being present") {
        REQUIRE(renderer_init == true);
    }

    SECTION("audio initialized despite renderer being present") {
        REQUIRE(audio_init == true);
    }

    bootstrapper_t::reset();
}
