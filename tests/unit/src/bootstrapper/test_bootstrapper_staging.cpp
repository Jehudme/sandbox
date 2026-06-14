// unit/src/bootstrapper/test_bootstrapper_staging.cpp
// Tests for Bootstrapper staging: stage_service, stage_module, reset.

#include <catch2/catch_all.hpp>
#include "core/bootstrapper.h"

using sandbox::core::Bootstrapper;
using sandbox::core::ServiceInfo;
using sandbox::core::ModuleInfo;

static ServiceInfo make_service(const char* name, int major, int minor) {
    ServiceInfo s{};
    s.name = name; s.description = "Test service"; s.architecture = "test_arch";
    s.version_major = major; s.version_minor = minor; s.init_fn = nullptr;
    return s;
}

static ModuleInfo make_module(const char* name, int major, int minor, int patch,
                              const ServiceInfo* svc = nullptr) {
    ModuleInfo m{};
    m.name = name; m.description = "Test module"; m.architecture = "test_arch";
    m.version_major = major; m.version_minor = minor; m.version_patch = patch;
    m.service = svc; m.requirements = nullptr; m.requirement_count = 0; m.init_fn = nullptr;
    return m;
}

TEST_CASE("Boot: stage_service and stage_module", "[bootstrapper][staging]")
{
    SECTION("staged service lets its module be activated") {
        Bootstrapper::reset();
        ServiceInfo svc = make_service("IRenderer", 1, 0);
        ModuleInfo  mod = make_module("OpenGLRenderer", 1, 0, 0, &svc);
        Bootstrapper::stage_service(svc);
        Bootstrapper::stage_module(mod);
        Bootstrapper b;
        REQUIRE_NOTHROW(b.activate("test_arch", "OpenGLRenderer", 1, 0, 0));
        Bootstrapper::reset();
    }

    SECTION("duplicate service staging is deduped") {
        Bootstrapper::reset();
        ServiceInfo svc = make_service("IPhysics", 2, 0);
        Bootstrapper::stage_service(svc);
        Bootstrapper::stage_service(svc);  // second call should be a no-op
        ModuleInfo mod = make_module("BulletPhysics", 2, 0, 0, &svc);
        Bootstrapper::stage_module(mod);
        Bootstrapper b;
        REQUIRE_NOTHROW(b.activate("test_arch", "BulletPhysics", 2, 0, 0));
        Bootstrapper::reset();
    }

    SECTION("staged module is activatable") {
        Bootstrapper::reset();
        ModuleInfo mod = make_module("AudioEngine", 1, 0, 0);
        Bootstrapper::stage_module(mod);
        Bootstrapper b;
        REQUIRE_NOTHROW(b.activate("test_arch", "AudioEngine", 1, 0, 0));
        Bootstrapper::reset();
    }

    SECTION("duplicate module staging is deduped") {
        Bootstrapper::reset();
        ModuleInfo mod = make_module("NetworkModule", 1, 2, 3);
        Bootstrapper::stage_module(mod);
        Bootstrapper::stage_module(mod);  // second call should be a no-op
        Bootstrapper b;
        REQUIRE_NOTHROW(b.activate("test_arch", "NetworkModule", 1, 2, 3));
        Bootstrapper::reset();
    }
}

TEST_CASE("Boot: reset() clears the registry", "[bootstrapper][staging]")
{
    SECTION("activate fails after reset") {
        ServiceInfo svc = make_service("IDummy", 1, 0);
        ModuleInfo  mod = make_module("DummyModule", 1, 0, 0, &svc);
        Bootstrapper::stage_service(svc);
        Bootstrapper::stage_module(mod);
        Bootstrapper::reset();
        Bootstrapper b;
        REQUIRE_THROWS_AS(b.activate("test_arch", "DummyModule", 1, 0, 0), std::invalid_argument);
    }

    SECTION("reset is safe to call multiple times on empty registry") {
        Bootstrapper::reset();
        REQUIRE_NOTHROW(Bootstrapper::reset());
        REQUIRE_NOTHROW(Bootstrapper::reset());
    }
}
