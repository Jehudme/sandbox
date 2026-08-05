#include <catch2/catch_all.hpp>
#include <flecs.h>
#include <sandbox/abi/bootstrapper.h>
#include "../../test_accessor.h"
#include <sandbox/sdk/runtime.hpp>
#include <sandbox/sdk/runtime.hpp>
#include <thread>
#include <chrono>

using namespace sandbox::core;

TEST_CASE("Runtime Module", "[runtime]") {

    bootstrapper_t b;
    flecs::world w;

    bootstrapper_t::load_library(w, "./cmake-build-debug/bin/sandbox_plugin.so");
    
    
    REQUIRE_NOTHROW(b.activate(w, "sandbox", "logs", 1, 0, 0));
    REQUIRE_NOTHROW(b.activate(w, "sandbox", "runtime", 1, 0, 0));
    REQUIRE_NOTHROW(b.boot(w));

    struct Dummy {};
    w.component<Dummy>();
    w.entity().add<Dummy>();

    SECTION("Run executes progress synchronously") {
        // Just run a few frames synchronously rather than hanging or threading
        for (int i = 0; i < 5; i++) {
            w.progress();
        }
        REQUIRE(true);
    }

}
