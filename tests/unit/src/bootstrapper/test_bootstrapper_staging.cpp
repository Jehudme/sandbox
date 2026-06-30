// unit/src/bootstrapper/test_bootstrapper_staging.cpp
// Tests for bootstrapper_t staging: stage_service, stage_module, reset.

#include <catch2/catch_all.hpp>
#include "core/bootstrapper.h"
#include "core/exceptions.h"

using sandbox::core::bootstrapper_t;
using sandbox::core::service_info_t;
using sandbox::core::module_info_t;

static service_info_t make_service(const char* name, int major, int minor) {
    service_info_t s{};
    s.name = name; s.description = "Test service"; s.architecture = "test_arch";
    s.version_major = major; s.version_minor = minor; s.init_fn = nullptr;
    return s;
}

static module_info_t make_module(const char* name, int major, int minor, int patch,
                              const service_info_t* svc = nullptr) {
    module_info_t m{};
    m.name = name; m.description = "Test module"; m.architecture = "test_arch";
    m.version_major = major; m.version_minor = minor; m.version_patch = patch;
    m.service = svc; m.requirements = nullptr; m.requirement_count = 0; m.init_fn = nullptr;
    return m;
}

TEST_CASE("Boot: stage_service and stage_module", "[bootstrapper][staging]")
{
    flecs::world ecs;
    SECTION("staged service lets its module be activated") {
        bootstrapper_t::reset();
        service_info_t svc = make_service("IRenderer", 1, 0);
        module_info_t  mod = make_module("OpenGLRenderer", 1, 0, 0, &svc);
        bootstrapper_t::stage_service(svc);
        bootstrapper_t::stage_module(mod);
        bootstrapper_t b;
        REQUIRE_NOTHROW(b.activate(ecs, "test_arch", "OpenGLRenderer", 1, 0, 0));
        bootstrapper_t::reset();
    }

    SECTION("duplicate service staging is deduped") {
        bootstrapper_t::reset();
        service_info_t svc = make_service("IPhysics", 2, 0);
        bootstrapper_t::stage_service(svc);
        bootstrapper_t::stage_service(svc);  // second call should be a no-op
        module_info_t mod = make_module("BulletPhysics", 2, 0, 0, &svc);
        bootstrapper_t::stage_module(mod);
        bootstrapper_t b;
        REQUIRE_NOTHROW(b.activate(ecs, "test_arch", "BulletPhysics", 2, 0, 0));
        bootstrapper_t::reset();
    }

    SECTION("staged module is activatable") {
        bootstrapper_t::reset();
        module_info_t mod = make_module("AudioEngine", 1, 0, 0);
        bootstrapper_t::stage_module(mod);
        bootstrapper_t b;
        REQUIRE_NOTHROW(b.activate(ecs, "test_arch", "AudioEngine", 1, 0, 0));
        bootstrapper_t::reset();
    }

    SECTION("duplicate module staging is deduped") {
        bootstrapper_t::reset();
        module_info_t mod = make_module("NetworkModule", 1, 2, 3);
        bootstrapper_t::stage_module(mod);
        bootstrapper_t::stage_module(mod);  // second call should be a no-op
        bootstrapper_t b;
        REQUIRE_NOTHROW(b.activate(ecs, "test_arch", "NetworkModule", 1, 2, 3));
        bootstrapper_t::reset();
    }
}

TEST_CASE("Boot: reset() clears the registry", "[bootstrapper][staging]")
{
    SECTION("activate fails after reset") {
        service_info_t svc = make_service("IDummy", 1, 0);
        module_info_t  mod = make_module("DummyModule", 1, 0, 0, &svc);
        bootstrapper_t::stage_service(svc);
        bootstrapper_t::stage_module(mod);
        bootstrapper_t::reset();
        bootstrapper_t b;
        flecs::world ecs;
        REQUIRE_THROWS_AS(b.activate(ecs, "test_arch", "DummyModule", 1, 0, 0), sandbox::core::module_activation_error);
    }

    SECTION("reset is safe to call multiple times on empty registry") {
        bootstrapper_t::reset();
        REQUIRE_NOTHROW(bootstrapper_t::reset());
        REQUIRE_NOTHROW(bootstrapper_t::reset());
    }
}
