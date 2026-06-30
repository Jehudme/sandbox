#include <catch2/catch_all.hpp>
#include <flecs.h>
#include <sandbox/abi/bootstrapper.h>
#include "core/bootstrapper.h"
#include <sandbox/sdk/runtime.hpp>
#include <sandbox/abi/runtime.h>
#include <thread>
#include <chrono>

using namespace sandbox::core;

TEST_CASE("Runtime Module", "[runtime]") {
    bootstrapper_t::reset();

    bootstrapper_t b;
    flecs::world w;

    b.index_library(w, "./logs.so");
    b.index_library(w, "./runtime.so");
    
    REQUIRE_NOTHROW(b.activate(w, "sandbox", "logs", 1, 0, 0));
    REQUIRE_NOTHROW(b.activate(w, "sandbox", "runtime", 1, 0, 0));
    REQUIRE_NOTHROW(b.boot(w));

    struct Dummy {};
    w.component<Dummy>();
    w.entity().add<Dummy>();

    SECTION("Run executes progress synchronously") {
        std::thread terminator([&]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            w.quit();
        });
         
        sandbox::modules::runtime::run(w);
        terminator.join();
        REQUIRE(true);
    }

    SECTION("Start executes progress in a separate thread") {
        sandbox::modules::runtime::start(w);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        w.quit();
        sandbox::modules::runtime::stop(w);
        
        REQUIRE(true);
    }

    bootstrapper_t::reset();
}
