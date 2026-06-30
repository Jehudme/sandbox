#include <catch2/catch_all.hpp>
#include <flecs.h>
#include "core/engine.h"
#include <sandbox/sdk/configuration.hpp>
#include <sandbox/sdk/logs.hpp>
#include <sandbox/abi/filesystem.h>
#include <sandbox/abi/bootstrapper.h>
#include "core/bootstrapper.h"
#include <fstream>
#include <string>

using namespace sandbox::core;

TEST_CASE("Filesystem Module", "[filesystem]") {
    bootstrapper_t::reset();

    // Create a dummy file to test mounting
    {
        std::ofstream dummy("test_physical_dir.txt");
        dummy << "hello";
    }

    // 1. Setup properties with mounts
    properties_t engine_props;
    engine_props.set<std::vector<std::string>>({"engine", "libraries"}, {"./configuration.so", "./logs.so", "./filesystem.so"});
    engine_props.set<std::vector<std::string>>({"engine", "sandbox"}, {"sandbox::core-configuration@1.0.0", "sandbox::core-logs@1.0.0", "sandbox::core-filesystem@1.0.0"});
    
    // Add filesystem mount config
    engine_props.set<std::string>({"filesystem", "mounts", "test_mount", "physical"}, "./");
    engine_props.set<bool>({"filesystem", "mounts", "test_mount", "readonly"}, true);

    // 2. Initialize engine
    engine_t engine;
    REQUIRE_NOTHROW(engine.initialize(engine_props));

    flecs::world& world = engine.ecs;

    // 3. Retrieve service and manually call mount to test API
    const sandbox_filesystem_service_t* svc = SANDBOX_GET_SERVICE(world, sandbox_filesystem_service_t);
    REQUIRE(svc != nullptr);
    REQUIRE(svc->api != nullptr);

    bool result = svc->api->mount(world.c_ptr(), "./test_physical_dir.txt", "/test", true);
    REQUIRE(result == true);

    bootstrapper_t::reset();
}
