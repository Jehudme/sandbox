#include <catch2/catch_test_macros.hpp>
#include "core/engine.h"
#include <sandbox/sdk/properties.hpp>
#include <sandbox/sdk/configuration.hpp>

using namespace sandbox;
using namespace sandbox::core;
using namespace sandbox::modules;

TEST_CASE("Configuration module initializes and provides SDK access", "[configuration][module]") {
    // 1. Create engine properties
    properties_t engine_props;
    engine_props.set<int64_t>({"test_key"}, 42);
    engine_props.set<std::string>({"test_str"}, "hello");
    engine_props.set<double>({"test_float"}, 3.14);
    
    // Tell engine to load our module dynamically
    std::vector<std::string> libs = {"./configuration.so"};
    std::vector<std::string> mods = {"sandbox-configuration@1.0.0"};
    engine_props.set<std::vector<std::string>>({"engine", "libraries"}, libs);
    engine_props.set<std::vector<std::string>>({"engine", "sandbox"}, mods);

    // 2. Initialize engine
    // Engine will index libraries, activate the configuration module,
    // push properties to Flecs, and then boot.
    engine_t engine;
    engine.initialize(engine_props);

    flecs::world& world = engine.ecs;
    
    // Wait, by the time boot() completes, the configuration module has loaded its Flecs C++ module,
    // picked up the handle from Flecs, and initialized itself!

    // 3. Test SDK reads
    auto val = configuration::get<int64_t>(world, "test_key");
    REQUIRE(val.has_value());
    REQUIRE(val.value() == 42);

    auto str = configuration::get<std::string>(world, "test_str");
    REQUIRE(str.has_value());
    REQUIRE(str.value() == "hello");

    auto flt = configuration::get<double>(world, "test_float");
    REQUIRE(flt.has_value());
    REQUIRE(flt.value() == 3.14);

    // 4. Test SDK writes
    configuration::set<int64_t>(world, "new_key", 100);
    auto new_val = configuration::get<int64_t>(world, "new_key");
    REQUIRE(new_val.has_value());
    REQUIRE(new_val.value() == 100);
}
