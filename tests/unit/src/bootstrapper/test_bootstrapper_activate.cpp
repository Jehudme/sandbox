// unit/src/bootstrapper/test_bootstrapper_activate.cpp
// Tests for bootstrapper_t::activate() — version matching and error cases.

#include <catch2/catch_all.hpp>
#include "core/bootstrapper.h"
#include "core/exceptions.h"

using sandbox::core::bootstrapper_t;
using sandbox::core::module_info_t;
using sandbox::core::service_info_t;

static module_info_t make_module(const char* name, const char* arch,
                               int major, int minor, int patch,
                               const service_info_t* svc = nullptr) {
    module_info_t m{};
    m.name = name; m.description = "Test"; m.architecture = arch;
    m.version_major = major; m.version_minor = minor; m.version_patch = patch;
    m.service = svc; m.requirements = nullptr; m.requirement_count = 0; m.init_fn = nullptr;
    return m;
}

TEST_CASE("Boot: activate() version resolution", "[bootstrapper][activate]")
{
    flecs::world ecs;
    SECTION("exact patch match succeeds") {
        bootstrapper_t::reset();
        bootstrapper_t::stage_module(make_module("Renderer", "sandbox::system", 1, 0, 0));
        bootstrapper_t b;
        REQUIRE_NOTHROW(b.activate(ecs, "sandbox::system", "Renderer", 1, 0, 0));
        bootstrapper_t::reset();
    }

    SECTION("patch -1 picks the highest available patch") {
        bootstrapper_t::reset();
        bootstrapper_t::stage_module(make_module("Renderer", "sandbox::system", 1, 0, 1));
        bootstrapper_t::stage_module(make_module("Renderer", "sandbox::system", 1, 0, 3));
        bootstrapper_t::stage_module(make_module("Renderer", "sandbox::system", 1, 0, 0));
        bootstrapper_t b;
        REQUIRE_NOTHROW(b.activate(ecs, "sandbox::system", "Renderer", 1, 0, -1));
        bootstrapper_t::reset();
    }

    SECTION("picks best minor when minor >= requested") {
        bootstrapper_t::reset();
        static int chosen = -1;
        chosen = -1;
        auto m110 = make_module("Service", "sandbox::system", 1, 1, 0);
        auto m150 = make_module("Service", "sandbox::system", 1, 5, 0);
        m110.init_fn = [](ecs_world_t*) { chosen = 1; };
        m150.init_fn = [](ecs_world_t*) { chosen = 5; };
        bootstrapper_t::stage_module(m110);
        bootstrapper_t::stage_module(m150);
        bootstrapper_t b;
        b.activate(ecs, "sandbox::system", "Service", 1, 0, -1);
        flecs::world w;
        b.boot(w);
        REQUIRE(chosen == 5);
        bootstrapper_t::reset();
    }

    SECTION("activate twice is a no-op") {
        bootstrapper_t::reset();
        static int calls = 0;
        calls = 0;
        auto m = make_module("Counter", "sandbox::system", 1, 0, 0);
        m.init_fn = [](ecs_world_t*) { calls++; };
        bootstrapper_t::stage_module(m);
        bootstrapper_t b;
        b.activate(ecs, "sandbox::system", "Counter", 1, 0, 0);
        b.activate(ecs, "sandbox::system", "Counter", 1, 0, 0);
        flecs::world w;
        b.boot(w);
        REQUIRE(calls == 1);
        bootstrapper_t::reset();
    }
}

TEST_CASE("Boot: activate() error cases", "[bootstrapper][activate]")
{
    flecs::world ecs;
    SECTION("unknown module name throws") {
        bootstrapper_t::reset();
        bootstrapper_t b;
        REQUIRE_THROWS_AS(
            b.activate(ecs, "sandbox::system", "NonExistent", 1, 0, 0),
            sandbox::core::module_activation_error
        );
        bootstrapper_t::reset();
    }

    SECTION("wrong architecture throws") {
        bootstrapper_t::reset();
        bootstrapper_t::stage_module(make_module("Physics", "arm64", 1, 0, 0));
        bootstrapper_t b;
        REQUIRE_THROWS_AS(
            b.activate(ecs, "sandbox::system", "Physics", 1, 0, 0),
            sandbox::core::module_activation_error
        );
        bootstrapper_t::reset();
    }

    SECTION("mismatched major version throws") {
        bootstrapper_t::reset();
        bootstrapper_t::stage_module(make_module("Engine", "sandbox::system", 2, 0, 0));
        bootstrapper_t b;
        REQUIRE_THROWS_AS(
            b.activate(ecs, "sandbox::system", "Engine", 3, 0, -1),
            sandbox::core::module_activation_error
        );
        bootstrapper_t::reset();
    }
}
