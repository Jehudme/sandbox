// suite/src/suite_complex_dependency_graph.cpp
// Integration: Complex module dependency graphs — diamond, transitive, and optional deps.

#include <catch2/catch_all.hpp>
#include "../../../sandbox/include/sandbox/abi/bootstrapper.h"
#include "core/bootstrapper.h"

#include <flecs.h>
#include <vector>
#include <string>
#include <algorithm>

using sandbox::core::bootstrapper_t;
using sandbox::core::module_info_t;

static std::vector<std::string> g_init_order;

static module_info_t make_mod(const char* name, int major, int minor, int patch,
                            void (*fn)(ecs_world_t*),
                            const sandbox_requirement_info_t* reqs = nullptr,
                            size_t req_count = 0) {
    module_info_t m{};
    m.name = name; m.description = "Complex dep"; m.architecture = "sandbox::system";
    m.version_major = major; m.version_minor = minor; m.version_patch = patch;
    m.service = nullptr; m.requirements = reqs; m.requirement_count = req_count;
    m.init_fn = fn;
    return m;
}

// ---------------------------------------------------------------------------
// Suite: Diamond dependency graph
// ---------------------------------------------------------------------------
TEST_CASE("Suite: Diamond dependency — correct order, no duplicates", "[suite][complex_dep]")
{
    bootstrapper_t::reset();
    g_init_order.clear();

    // Foundation → Math → {Physics, AI} → GameWorld
    auto foundation = make_mod("Foundation", 1, 0, 0,
        [](ecs_world_t*) { g_init_order.push_back("Foundation"); });

    sandbox_requirement_info_t req_foundation[] = {{
        SANDBOX_REQUIREMENT_KIND_MODULE, SANDBOX_REQUIREMENT_STRICTNESS_REQUIRED,
        "Foundation", "sandbox::system", 1, 0, -1
    }};
    auto math = make_mod("Math", 1, 0, 0,
        [](ecs_world_t*) { g_init_order.push_back("Math"); },
        req_foundation, 1);

    sandbox_requirement_info_t req_math[] = {{
        SANDBOX_REQUIREMENT_KIND_MODULE, SANDBOX_REQUIREMENT_STRICTNESS_REQUIRED,
        "Math", "sandbox::system", 1, 0, -1
    }};
    auto physics = make_mod("Physics", 1, 0, 0,
        [](ecs_world_t*) { g_init_order.push_back("Physics"); },
        req_math, 1);
    auto ai = make_mod("AI", 1, 0, 0,
        [](ecs_world_t*) { g_init_order.push_back("AI"); },
        req_math, 1);

    sandbox_requirement_info_t req_world[] = {
        { SANDBOX_REQUIREMENT_KIND_MODULE, SANDBOX_REQUIREMENT_STRICTNESS_REQUIRED,
          "Physics", "sandbox::system", 1, 0, -1 },
        { SANDBOX_REQUIREMENT_KIND_MODULE, SANDBOX_REQUIREMENT_STRICTNESS_REQUIRED,
          "AI", "sandbox::system", 1, 0, -1 },
    };
    auto world = make_mod("GameWorld", 1, 0, 0,
        [](ecs_world_t*) { g_init_order.push_back("GameWorld"); },
        req_world, 2);

    bootstrapper_t::stage_module(foundation);
    bootstrapper_t::stage_module(math);
    bootstrapper_t::stage_module(physics);
    bootstrapper_t::stage_module(ai);
    bootstrapper_t::stage_module(world);

    bootstrapper_t b;
    b.activate("sandbox::system", "GameWorld", 1, 0, 0);
    flecs::world w;
    REQUIRE_NOTHROW(b.boot(w));

    SECTION("all 5 sandbox were initialized") {
        REQUIRE(g_init_order.size() == 5);
    }

    SECTION("Math initialized exactly once (diamond dedup)") {
        REQUIRE(std::count(g_init_order.begin(), g_init_order.end(), "Math") == 1);
    }

    SECTION("Foundation boots before Math") {
        auto f = std::find(g_init_order.begin(), g_init_order.end(), "Foundation");
        auto m = std::find(g_init_order.begin(), g_init_order.end(), "Math");
        REQUIRE(f < m);
    }

    SECTION("Math boots before Physics and AI") {
        auto m  = std::find(g_init_order.begin(), g_init_order.end(), "Math");
        auto ph = std::find(g_init_order.begin(), g_init_order.end(), "Physics");
        auto ai = std::find(g_init_order.begin(), g_init_order.end(), "AI");
        REQUIRE(m < ph);
        REQUIRE(m < ai);
    }

    SECTION("GameWorld boots last") {
        REQUIRE(g_init_order.back() == "GameWorld");
    }

    bootstrapper_t::reset();
}

// ---------------------------------------------------------------------------
// Suite: Optional (expected) dependency
// ---------------------------------------------------------------------------
TEST_CASE("Suite: Optional dependency — present and absent", "[suite][complex_dep]")
{
    auto core_mod = make_mod("CoreModule", 1, 0, 0,
        [](ecs_world_t*) { g_init_order.push_back("CoreModule"); });
    auto debug_mod = make_mod("DebugModule", 1, 0, 0,
        [](ecs_world_t*) { g_init_order.push_back("DebugModule"); });

    sandbox_requirement_info_t app_reqs[] = {
        { SANDBOX_REQUIREMENT_KIND_MODULE, SANDBOX_REQUIREMENT_STRICTNESS_REQUIRED,
          "CoreModule", "sandbox::system", 1, 0, -1 },
        { SANDBOX_REQUIREMENT_KIND_MODULE, SANDBOX_REQUIREMENT_STRICTNESS_EXPECTED,
          "DebugModule", "sandbox::system", 1, 0, -1 },
    };
    auto app_mod = make_mod("AppModule", 1, 0, 0,
        [](ecs_world_t*) { g_init_order.push_back("AppModule"); },
        app_reqs, 2);

    SECTION("optional dep present — all three sandbox initialize") {
        bootstrapper_t::reset();
        g_init_order.clear();
        bootstrapper_t::stage_module(core_mod);
        bootstrapper_t::stage_module(debug_mod);
        bootstrapper_t::stage_module(app_mod);
        bootstrapper_t b;
        b.activate("sandbox::system", "AppModule", 1, 0, 0);
        flecs::world w;
        REQUIRE_NOTHROW(b.boot(w));
        REQUIRE(g_init_order.size() == 3);
        REQUIRE(std::find(g_init_order.begin(), g_init_order.end(), "DebugModule") != g_init_order.end());
        bootstrapper_t::reset();
    }

    SECTION("optional dep absent — only sdk and app initialize") {
        bootstrapper_t::reset();
        g_init_order.clear();
        bootstrapper_t::stage_module(core_mod);
        // DebugModule NOT staged
        bootstrapper_t::stage_module(app_mod);
        bootstrapper_t b;
        b.activate("sandbox::system", "AppModule", 1, 0, 0);
        flecs::world w;
        REQUIRE_NOTHROW(b.boot(w));
        REQUIRE(g_init_order.size() == 2);
        REQUIRE(std::find(g_init_order.begin(), g_init_order.end(), "DebugModule") == g_init_order.end());
        bootstrapper_t::reset();
    }
}
