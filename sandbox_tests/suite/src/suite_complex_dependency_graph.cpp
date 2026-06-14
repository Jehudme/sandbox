// suite/src/suite_complex_dependency_graph.cpp
//
// Integration suite: Complex dependency graph
//
// Tests multi-level, transitive dependency chains and diamond-dependency
// patterns (A→B, A→C, B→D, C→D — D should boot once and first).
//
// Layout:
//   FoundationModule (no deps)
//       └── MathModule (requires FoundationModule)
//           ├── PhysicsModule (requires MathModule)
//           └── AIModule      (requires MathModule)
//               └── GameWorldModule (requires PhysicsModule + AIModule)
//
// This exercises:
//   - 3-level transitive resolution
//   - Diamond dependency (MathModule pulled once despite two consumers)
//   - Correct topological init order

#include <catch2/catch_all.hpp>
#include "sandbox/core/bootstrapper.h"
#include "core/bootstrapper.h"

#include <flecs.h>
#include <vector>
#include <string>
#include <algorithm>

using sandbox::core::Bootstrapper;
using sandbox::core::ModuleInfo;

// ---------------------------------------------------------------------------
// Global init-order tracker
// ---------------------------------------------------------------------------
static std::vector<std::string> g_complex_init_order;

static void reset_complex_init_order() {
    g_complex_init_order.clear();
}

// ---------------------------------------------------------------------------
// Module definitions (built manually — not via macro — to control init_fn)
// ---------------------------------------------------------------------------

static ModuleInfo make_complex_module(
    const char* name,
    int major, int minor, int patch,
    void (*init_fn)(ecs_world_t*),
    const sandbox_requirement_info_t* requirements = nullptr,
    size_t requirement_count = 0
) {
    ModuleInfo module_info{};
    module_info.name              = name;
    module_info.description       = "Complex dep graph module";
    module_info.architecture      = "x86_64";
    module_info.version_major     = major;
    module_info.version_minor     = minor;
    module_info.version_patch     = patch;
    module_info.service           = nullptr;
    module_info.requirements      = requirements;
    module_info.requirement_count = requirement_count;
    module_info.init_fn           = init_fn;
    return module_info;
}

// ---------------------------------------------------------------------------
// Suite: Diamond dependency graph
// ---------------------------------------------------------------------------
TEST_CASE("Suite: Complex dependency graph — diamond pattern resolves each module once",
          "[suite][complex_dep][diamond]")
{
    Bootstrapper::reset();
    reset_complex_init_order();

    // FoundationModule — no dependencies
    ModuleInfo foundation_module = make_complex_module("FoundationModule", 1, 0, 0,
        [](ecs_world_t*) { g_complex_init_order.push_back("FoundationModule"); });

    // MathModule — requires FoundationModule
    sandbox_requirement_info_t math_requires[] = {{
        .kind          = SANDBOX_REQUIREMENT_KIND_MODULE,
        .strictness    = SANDBOX_REQUIREMENT_STRICTNESS_REQUIRED,
        .name          = "FoundationModule",
        .architecture  = "x86_64",
        .version_major = 1,
        .version_minor = 0,
        .version_patch = -1,
    }};

    ModuleInfo math_module = make_complex_module("MathModule", 1, 0, 0,
        [](ecs_world_t*) { g_complex_init_order.push_back("MathModule"); },
        math_requires, 1);

    // PhysicsModule — requires MathModule
    sandbox_requirement_info_t physics_requires[] = {{
        .kind          = SANDBOX_REQUIREMENT_KIND_MODULE,
        .strictness    = SANDBOX_REQUIREMENT_STRICTNESS_REQUIRED,
        .name          = "MathModule",
        .architecture  = "x86_64",
        .version_major = 1,
        .version_minor = 0,
        .version_patch = -1,
    }};

    ModuleInfo physics_module = make_complex_module("PhysicsModule", 1, 0, 0,
        [](ecs_world_t*) { g_complex_init_order.push_back("PhysicsModule"); },
        physics_requires, 1);

    // AIModule — requires MathModule (diamond: both Physics and AI need Math)
    sandbox_requirement_info_t ai_requires[] = {{
        .kind          = SANDBOX_REQUIREMENT_KIND_MODULE,
        .strictness    = SANDBOX_REQUIREMENT_STRICTNESS_REQUIRED,
        .name          = "MathModule",
        .architecture  = "x86_64",
        .version_major = 1,
        .version_minor = 0,
        .version_patch = -1,
    }};

    ModuleInfo ai_module = make_complex_module("AIModule", 1, 0, 0,
        [](ecs_world_t*) { g_complex_init_order.push_back("AIModule"); },
        ai_requires, 1);

    // GameWorldModule — requires PhysicsModule + AIModule
    sandbox_requirement_info_t game_world_requires[] = {
        {
            .kind          = SANDBOX_REQUIREMENT_KIND_MODULE,
            .strictness    = SANDBOX_REQUIREMENT_STRICTNESS_REQUIRED,
            .name          = "PhysicsModule",
            .architecture  = "x86_64",
            .version_major = 1,
            .version_minor = 0,
            .version_patch = -1,
        },
        {
            .kind          = SANDBOX_REQUIREMENT_KIND_MODULE,
            .strictness    = SANDBOX_REQUIREMENT_STRICTNESS_REQUIRED,
            .name          = "AIModule",
            .architecture  = "x86_64",
            .version_major = 1,
            .version_minor = 0,
            .version_patch = -1,
        },
    };

    ModuleInfo game_world_module = make_complex_module("GameWorldModule", 1, 0, 0,
        [](ecs_world_t*) { g_complex_init_order.push_back("GameWorldModule"); },
        game_world_requires, 2);

    // Stage all
    Bootstrapper::stage_module(foundation_module);
    Bootstrapper::stage_module(math_module);
    Bootstrapper::stage_module(physics_module);
    Bootstrapper::stage_module(ai_module);
    Bootstrapper::stage_module(game_world_module);

    // Only activate the top-level module
    Bootstrapper bootstrapper_instance;
    bootstrapper_instance.activate("x86_64", "GameWorldModule", 1, 0, 0);

    flecs::world test_world;
    REQUIRE_NOTHROW(bootstrapper_instance.boot(test_world));

    SECTION("FoundationModule was initialized") {
        REQUIRE(std::find(g_complex_init_order.begin(), g_complex_init_order.end(),
                          "FoundationModule") != g_complex_init_order.end());
    }

    SECTION("MathModule was initialized exactly once (diamond dedup)") {
        size_t math_count = std::count(g_complex_init_order.begin(), g_complex_init_order.end(), "MathModule");
        REQUIRE(math_count == 1);
    }

    SECTION("PhysicsModule was initialized") {
        REQUIRE(std::find(g_complex_init_order.begin(), g_complex_init_order.end(),
                          "PhysicsModule") != g_complex_init_order.end());
    }

    SECTION("AIModule was initialized") {
        REQUIRE(std::find(g_complex_init_order.begin(), g_complex_init_order.end(),
                          "AIModule") != g_complex_init_order.end());
    }

    SECTION("GameWorldModule was initialized last") {
        REQUIRE(g_complex_init_order.back() == "GameWorldModule");
    }

    SECTION("FoundationModule boots before MathModule") {
        auto foundation_pos = std::find(g_complex_init_order.begin(), g_complex_init_order.end(), "FoundationModule");
        auto math_pos       = std::find(g_complex_init_order.begin(), g_complex_init_order.end(), "MathModule");
        REQUIRE(foundation_pos < math_pos);
    }

    SECTION("MathModule boots before PhysicsModule") {
        auto math_pos    = std::find(g_complex_init_order.begin(), g_complex_init_order.end(), "MathModule");
        auto physics_pos = std::find(g_complex_init_order.begin(), g_complex_init_order.end(), "PhysicsModule");
        REQUIRE(math_pos < physics_pos);
    }

    SECTION("MathModule boots before AIModule") {
        auto math_pos = std::find(g_complex_init_order.begin(), g_complex_init_order.end(), "MathModule");
        auto ai_pos   = std::find(g_complex_init_order.begin(), g_complex_init_order.end(), "AIModule");
        REQUIRE(math_pos < ai_pos);
    }

    SECTION("PhysicsModule boots before GameWorldModule") {
        auto physics_pos    = std::find(g_complex_init_order.begin(), g_complex_init_order.end(), "PhysicsModule");
        auto game_world_pos = std::find(g_complex_init_order.begin(), g_complex_init_order.end(), "GameWorldModule");
        REQUIRE(physics_pos < game_world_pos);
    }

    SECTION("AIModule boots before GameWorldModule") {
        auto ai_pos         = std::find(g_complex_init_order.begin(), g_complex_init_order.end(), "AIModule");
        auto game_world_pos = std::find(g_complex_init_order.begin(), g_complex_init_order.end(), "GameWorldModule");
        REQUIRE(ai_pos < game_world_pos);
    }

    SECTION("total init count is exactly 5 (no duplicates)") {
        REQUIRE(g_complex_init_order.size() == 5);
    }

    Bootstrapper::reset();
}

// ---------------------------------------------------------------------------
// Suite: Mixed required + expected dependency chain
// ---------------------------------------------------------------------------
TEST_CASE("Suite: Complex dependency — mixed required and expected deps",
          "[suite][complex_dep][mixed]")
{
    Bootstrapper::reset();
    reset_complex_init_order();

    // CoreModule — no deps, always present
    ModuleInfo core_module = make_complex_module("CoreModule", 1, 0, 0,
        [](ecs_world_t*) { g_complex_init_order.push_back("CoreModule"); });

    // DebugModule — optional (expected), enhances CoreModule
    ModuleInfo debug_module = make_complex_module("DebugModule", 1, 0, 0,
        [](ecs_world_t*) { g_complex_init_order.push_back("DebugModule"); });

    // AppModule — requires CoreModule, expects DebugModule (optional)
    sandbox_requirement_info_t app_requires[] = {
        {
            .kind          = SANDBOX_REQUIREMENT_KIND_MODULE,
            .strictness    = SANDBOX_REQUIREMENT_STRICTNESS_REQUIRED,
            .name          = "CoreModule",
            .architecture  = "x86_64",
            .version_major = 1,
            .version_minor = 0,
            .version_patch = -1,
        },
        {
            .kind          = SANDBOX_REQUIREMENT_KIND_MODULE,
            .strictness    = SANDBOX_REQUIREMENT_STRICTNESS_EXPECTED,
            .name          = "DebugModule",
            .architecture  = "x86_64",
            .version_major = 1,
            .version_minor = 0,
            .version_patch = -1,
        },
    };

    ModuleInfo app_module_with_debug = make_complex_module("AppModule", 1, 0, 0,
        [](ecs_world_t*) { g_complex_init_order.push_back("AppModule"); },
        app_requires, 2);

    // Scenario A: Debug IS available
    SECTION("optional DebugModule IS present — both core and debug are initialized") {
        Bootstrapper::stage_module(core_module);
        Bootstrapper::stage_module(debug_module);
        Bootstrapper::stage_module(app_module_with_debug);

        Bootstrapper bootstrapper_instance;
        bootstrapper_instance.activate("x86_64", "AppModule", 1, 0, 0);

        flecs::world test_world;
        REQUIRE_NOTHROW(bootstrapper_instance.boot(test_world));

        REQUIRE(std::find(g_complex_init_order.begin(), g_complex_init_order.end(), "CoreModule")  != g_complex_init_order.end());
        REQUIRE(std::find(g_complex_init_order.begin(), g_complex_init_order.end(), "DebugModule") != g_complex_init_order.end());
        REQUIRE(std::find(g_complex_init_order.begin(), g_complex_init_order.end(), "AppModule")   != g_complex_init_order.end());
        REQUIRE(g_complex_init_order.size() == 3);

        Bootstrapper::reset();
    }

    // Scenario B: Debug NOT available
    reset_complex_init_order();

    SECTION("optional DebugModule is ABSENT — only core and app are initialized") {
        Bootstrapper::stage_module(core_module);
        // debug NOT staged
        Bootstrapper::stage_module(app_module_with_debug);

        Bootstrapper bootstrapper_instance;
        bootstrapper_instance.activate("x86_64", "AppModule", 1, 0, 0);

        flecs::world test_world;
        REQUIRE_NOTHROW(bootstrapper_instance.boot(test_world));

        REQUIRE(std::find(g_complex_init_order.begin(), g_complex_init_order.end(), "CoreModule")  != g_complex_init_order.end());
        REQUIRE(std::find(g_complex_init_order.begin(), g_complex_init_order.end(), "DebugModule") == g_complex_init_order.end());
        REQUIRE(std::find(g_complex_init_order.begin(), g_complex_init_order.end(), "AppModule")   != g_complex_init_order.end());
        REQUIRE(g_complex_init_order.size() == 2);

        Bootstrapper::reset();
    }
}
