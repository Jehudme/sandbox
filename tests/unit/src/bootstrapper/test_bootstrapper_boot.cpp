// unit/src/bootstrapper/test_bootstrapper_boot.cpp
// Tests for bootstrapper_t::boot() — dependency resolution and error cases.

#include <catch2/catch_all.hpp>
#include "core/bootstrapper.h"

#include <vector>
#include <string>

using sandbox::core::bootstrapper_t;
using sandbox::core::module_info_t;
using sandbox::core::service_info_t;

static service_info_t make_service(const char* name, int major, int minor,
                                 void (*fn)(ecs_world_t*) = nullptr) {
    service_info_t s{};
    s.name = name; s.description = "svc"; s.architecture = "sandbox::system";
    s.version_major = major; s.version_minor = minor; s.init_fn = fn;
    return s;
}

static module_info_t make_mod(const char* name, int major, int minor, int patch,
                            void (*fn)(ecs_world_t*) = nullptr) {
    module_info_t m{};
    m.name = name; m.description = "mod"; m.architecture = "sandbox::system";
    m.version_major = major; m.version_minor = minor; m.version_patch = patch;
    m.service = nullptr; m.requirements = nullptr; m.requirement_count = 0; m.init_fn = fn;
    return m;
}

TEST_CASE("Boot: boot() initializes sandbox", "[bootstrapper][boot]")
{
    flecs::world ecs;
    SECTION("sandbox with no dependencies are all initialized") {
        bootstrapper_t::reset();
        static int a_calls = 0, b_calls = 0;
        a_calls = b_calls = 0;
        auto alpha = make_mod("Alpha", 1, 0, 0, [](ecs_world_t*) { a_calls++; });
        auto beta  = make_mod("Beta",  1, 0, 0, [](ecs_world_t*) { b_calls++; });
        bootstrapper_t::stage_module(alpha);
        bootstrapper_t::stage_module(beta);
        bootstrapper_t b;
        b.activate(ecs, "sandbox::system", "Alpha", 1, 0, 0);
        b.activate(ecs, "sandbox::system", "Beta",  1, 0, 0);
        flecs::world w;
        b.boot(w);
        REQUIRE(a_calls == 1);
        REQUIRE(b_calls == 1);
        bootstrapper_t::reset();
    }

    SECTION("required service dependency is auto-pulled") {
        bootstrapper_t::reset();
        static std::vector<std::string> order;
        order.clear();

        service_info_t logger_svc = make_service("ILogger", 1, 0,
            [](ecs_world_t*) { order.push_back("ILogger"); });
        auto provider = make_mod("LoggerImpl", 1, 0, 0,
            [](ecs_world_t*) { order.push_back("LoggerImpl"); });
        provider.service = &logger_svc;

        sandbox_requirement_info_t req[1];
        req[0] = { SANDBOX_REQUIREMENT_KIND_SERVICE, SANDBOX_REQUIREMENT_STRICTNESS_REQUIRED,
                   "ILogger", "sandbox::system", 1, 0, -1 };

        auto consumer = make_mod("Consumer", 1, 0, 0,
            [](ecs_world_t*) { order.push_back("Consumer"); });
        consumer.requirements = req;
        consumer.requirement_count = 1;

        bootstrapper_t::stage_module(provider);
        bootstrapper_t::stage_module(consumer);
        bootstrapper_t b;
        b.activate(ecs, "sandbox::system", "Consumer", 1, 0, 0);
        flecs::world w;
        REQUIRE_NOTHROW(b.boot(w));

        auto prov_pos = std::find(order.begin(), order.end(), "LoggerImpl");
        auto cons_pos = std::find(order.begin(), order.end(), "Consumer");
        REQUIRE(prov_pos != order.end());
        REQUIRE(cons_pos != order.end());
        REQUIRE(prov_pos < cons_pos);
        bootstrapper_t::reset();
    }

    SECTION("required module dependency is auto-pulled") {
        bootstrapper_t::reset();
        static bool dep_init = false, cons_init = false;
        dep_init = cons_init = false;

        auto dep = make_mod("MathLib", 1, 0, 0, [](ecs_world_t*) { dep_init = true; });
        sandbox_requirement_info_t req[1];
        req[0] = { SANDBOX_REQUIREMENT_KIND_MODULE, SANDBOX_REQUIREMENT_STRICTNESS_REQUIRED,
                   "MathLib", "sandbox::system", 1, 0, -1 };

        auto physics = make_mod("Physics", 1, 0, 0, [](ecs_world_t*) { cons_init = true; });
        physics.requirements = req;
        physics.requirement_count = 1;

        bootstrapper_t::stage_module(dep);
        bootstrapper_t::stage_module(physics);
        bootstrapper_t b;
        b.activate(ecs, "sandbox::system", "Physics", 1, 0, 0);
        flecs::world w;
        REQUIRE_NOTHROW(b.boot(w));
        REQUIRE(dep_init == true);
        REQUIRE(cons_init == true);
        bootstrapper_t::reset();
    }

    SECTION("optional (expected) dep is skipped when absent") {
        bootstrapper_t::reset();
        static bool initialized = false;
        initialized = false;

        sandbox_requirement_info_t req[1];
        req[0] = { SANDBOX_REQUIREMENT_KIND_MODULE, SANDBOX_REQUIREMENT_STRICTNESS_EXPECTED,
                   "OptionalModule", "sandbox::system", 1, 0, -1 };

        auto mod = make_mod("Flexible", 1, 0, 0, [](ecs_world_t*) { initialized = true; });
        mod.requirements = req;
        mod.requirement_count = 1;

        bootstrapper_t::stage_module(mod);
        bootstrapper_t b;
        b.activate(ecs, "sandbox::system", "Flexible", 1, 0, 0);
        flecs::world w;
        REQUIRE_NOTHROW(b.boot(w));
        REQUIRE(initialized == true);
        bootstrapper_t::reset();
    }

    SECTION("service collision evicts the lower-version provider") {
        bootstrapper_t::reset();
        static int winner = 0, loser = 0;
        winner = loser = 0;

        service_info_t svc_v1 = make_service("IRenderer", 1, 0);
        service_info_t svc_v2 = make_service("IRenderer", 2, 0);

        auto low  = make_mod("OpenGLv1", 1, 0, 0, [](ecs_world_t*) { loser++; });
        auto high = make_mod("Vulkan",   2, 0, 0, [](ecs_world_t*) { winner++; });
        low.service  = &svc_v1;
        high.service = &svc_v2;

        bootstrapper_t::stage_module(low);
        bootstrapper_t::stage_module(high);
        bootstrapper_t b;
        b.activate(ecs, "sandbox::system", "OpenGLv1", 1, 0, 0);
        b.activate(ecs, "sandbox::system", "Vulkan",   2, 0, 0);
        flecs::world w;
        REQUIRE_NOTHROW(b.boot(w));
        REQUIRE(winner == 1);
        REQUIRE(loser == 0);
        bootstrapper_t::reset();
    }
}

TEST_CASE("Boot: boot() error cases", "[bootstrapper][boot]")
{
    flecs::world ecs;
    SECTION("missing required service throws") {
        bootstrapper_t::reset();
        sandbox_requirement_info_t req[1];
        req[0] = { SANDBOX_REQUIREMENT_KIND_SERVICE, SANDBOX_REQUIREMENT_STRICTNESS_REQUIRED,
                   "INonExistent", "sandbox::system", 1, 0, -1 };
        auto mod = make_mod("Needy", 1, 0, 0);
        mod.requirements = req;
        mod.requirement_count = 1;
        bootstrapper_t::stage_module(mod);
        bootstrapper_t b;
        b.activate(ecs, "sandbox::system", "Needy", 1, 0, 0);
        flecs::world w;
        REQUIRE_THROWS_AS(b.boot(w), std::runtime_error);
        bootstrapper_t::reset();
    }

    SECTION("cyclic dependency throws") {
        bootstrapper_t::reset();
        sandbox_requirement_info_t req_b[1], req_a[1];
        req_b[0] = { SANDBOX_REQUIREMENT_KIND_MODULE, SANDBOX_REQUIREMENT_STRICTNESS_REQUIRED,
                     "ModuleB", "sandbox::system", 0, 0, -1 };
        req_a[0] = { SANDBOX_REQUIREMENT_KIND_MODULE, SANDBOX_REQUIREMENT_STRICTNESS_REQUIRED,
                     "ModuleA", "sandbox::system", 0, 0, -1 };

        auto ma = make_mod("ModuleA", 1, 0, 0);
        ma.requirements = req_b; ma.requirement_count = 1;
        auto mb = make_mod("ModuleB", 1, 0, 0);
        mb.requirements = req_a; mb.requirement_count = 1;

        bootstrapper_t::stage_module(ma);
        bootstrapper_t::stage_module(mb);
        bootstrapper_t b;
        b.activate(ecs, "sandbox::system", "ModuleA", 1, 0, 0);
        b.activate(ecs, "sandbox::system", "ModuleB", 1, 0, 0);
        flecs::world w;
        REQUIRE_THROWS_AS(b.boot(w), std::runtime_error);
        bootstrapper_t::reset();
    }

    SECTION("consumers disagreeing on service major version throws") {
        bootstrapper_t::reset();
        service_info_t svc_v1 = make_service("ILogger", 1, 0);
        service_info_t svc_v2 = make_service("ILogger", 2, 0);

        auto prov1 = make_mod("LoggerV1", 1, 0, 0); prov1.service = &svc_v1;
        auto prov2 = make_mod("LoggerV2", 2, 0, 0); prov2.service = &svc_v2;

        sandbox_requirement_info_t rq1[1], rq2[1];
        rq1[0] = { SANDBOX_REQUIREMENT_KIND_SERVICE, SANDBOX_REQUIREMENT_STRICTNESS_REQUIRED,
                   "ILogger", "sandbox::system", 1, 0, -1 };
        rq2[0] = { SANDBOX_REQUIREMENT_KIND_SERVICE, SANDBOX_REQUIREMENT_STRICTNESS_REQUIRED,
                   "ILogger", "sandbox::system", 2, 0, -1 };

        auto cons_a = make_mod("ConsumerA", 1, 0, 0);
        cons_a.requirements = rq1; cons_a.requirement_count = 1;
        auto cons_b = make_mod("ConsumerB", 1, 0, 0);
        cons_b.requirements = rq2; cons_b.requirement_count = 1;

        bootstrapper_t::stage_module(prov1);
        bootstrapper_t::stage_module(prov2);
        bootstrapper_t::stage_module(cons_a);
        bootstrapper_t::stage_module(cons_b);

        bootstrapper_t b;
        b.activate(ecs, "sandbox::system", "ConsumerA", 1, 0, 0);
        b.activate(ecs, "sandbox::system", "ConsumerB", 1, 0, 0);
        flecs::world w;
        REQUIRE_THROWS_AS(b.boot(w), std::runtime_error);
        bootstrapper_t::reset();
    }
}
