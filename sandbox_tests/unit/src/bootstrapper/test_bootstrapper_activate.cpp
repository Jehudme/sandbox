// unit/src/bootstrapper/test_bootstrapper_activate.cpp
// Tests for Bootstrapper::activate() — version matching and error cases.

#include <catch2/catch_all.hpp>
#include "core/bootstrapper.h"

using sandbox::core::Bootstrapper;
using sandbox::core::ModuleInfo;
using sandbox::core::ServiceInfo;

static ModuleInfo make_module(const char* name, const char* arch,
                               int major, int minor, int patch,
                               const ServiceInfo* svc = nullptr) {
    ModuleInfo m{};
    m.name = name; m.description = "Test"; m.architecture = arch;
    m.version_major = major; m.version_minor = minor; m.version_patch = patch;
    m.service = svc; m.requirements = nullptr; m.requirement_count = 0; m.init_fn = nullptr;
    return m;
}

TEST_CASE("Boot: activate() version resolution", "[bootstrapper][activate]")
{
    SECTION("exact patch match succeeds") {
        Bootstrapper::reset();
        Bootstrapper::stage_module(make_module("Renderer", "sandbox::system", 1, 0, 0));
        Bootstrapper b;
        REQUIRE_NOTHROW(b.activate("sandbox::system", "Renderer", 1, 0, 0));
        Bootstrapper::reset();
    }

    SECTION("patch -1 picks the highest available patch") {
        Bootstrapper::reset();
        Bootstrapper::stage_module(make_module("Renderer", "sandbox::system", 1, 0, 1));
        Bootstrapper::stage_module(make_module("Renderer", "sandbox::system", 1, 0, 3));
        Bootstrapper::stage_module(make_module("Renderer", "sandbox::system", 1, 0, 0));
        Bootstrapper b;
        REQUIRE_NOTHROW(b.activate("sandbox::system", "Renderer", 1, 0, -1));
        Bootstrapper::reset();
    }

    SECTION("picks best minor when minor >= requested") {
        Bootstrapper::reset();
        static int chosen = -1;
        chosen = -1;
        auto m110 = make_module("Service", "sandbox::system", 1, 1, 0);
        auto m150 = make_module("Service", "sandbox::system", 1, 5, 0);
        m110.init_fn = [](ecs_world_t*) { chosen = 1; };
        m150.init_fn = [](ecs_world_t*) { chosen = 5; };
        Bootstrapper::stage_module(m110);
        Bootstrapper::stage_module(m150);
        Bootstrapper b;
        b.activate("sandbox::system", "Service", 1, 0, -1);
        flecs::world w;
        b.boot(w);
        REQUIRE(chosen == 5);
        Bootstrapper::reset();
    }

    SECTION("activate twice is a no-op") {
        Bootstrapper::reset();
        static int calls = 0;
        calls = 0;
        auto m = make_module("Counter", "sandbox::system", 1, 0, 0);
        m.init_fn = [](ecs_world_t*) { calls++; };
        Bootstrapper::stage_module(m);
        Bootstrapper b;
        b.activate("sandbox::system", "Counter", 1, 0, 0);
        b.activate("sandbox::system", "Counter", 1, 0, 0);
        flecs::world w;
        b.boot(w);
        REQUIRE(calls == 1);
        Bootstrapper::reset();
    }
}

TEST_CASE("Boot: activate() error cases", "[bootstrapper][activate]")
{
    SECTION("unknown module name throws") {
        Bootstrapper::reset();
        Bootstrapper b;
        REQUIRE_THROWS_AS(
            b.activate("sandbox::system", "NonExistent", 1, 0, 0),
            std::invalid_argument
        );
        Bootstrapper::reset();
    }

    SECTION("wrong architecture throws") {
        Bootstrapper::reset();
        Bootstrapper::stage_module(make_module("Physics", "arm64", 1, 0, 0));
        Bootstrapper b;
        REQUIRE_THROWS_AS(
            b.activate("sandbox::system", "Physics", 1, 0, 0),
            std::invalid_argument
        );
        Bootstrapper::reset();
    }

    SECTION("mismatched major version throws") {
        Bootstrapper::reset();
        Bootstrapper::stage_module(make_module("Engine", "sandbox::system", 2, 0, 0));
        Bootstrapper b;
        REQUIRE_THROWS_AS(
            b.activate("sandbox::system", "Engine", 3, 0, -1),
            std::invalid_argument
        );
        Bootstrapper::reset();
    }
}
