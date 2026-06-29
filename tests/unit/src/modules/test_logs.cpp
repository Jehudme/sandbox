#include <catch2/catch_test_macros.hpp>
#include "core/engine.h"
#include <sandbox/sdk/properties.hpp>
#include <sandbox/sdk/configuration.hpp>
#include <sandbox/sdk/logs.hpp>
#include <fstream>
#include <sstream>

using namespace sandbox;
using namespace sandbox::core;
using namespace sandbox::modules;


TEST_CASE("Logs module end-to-end", "[logs][module]") {
    // 1. Create engine properties
    properties_t engine_props;
    
    // Configure logger
    engine_props.set<std::string>({"logs", "name"}, "test_logger");
    engine_props.set<std::string>({"logs", "level"}, "trace");
    engine_props.set<bool>({"logs", "console", "enabled"}, false); // Disable console for clean tests
    engine_props.set<std::string>({"logs", "file", "path"}, "test_log_output.txt");
    engine_props.set<bool>({"logs", "file", "truncate"}, true);
    
    // Tell engine to load our modules dynamically
    std::vector<std::string> libs = {"./configuration.so", "./logs.so"};
    std::vector<std::string> mods = {"sandbox-configuration@1.0.0", "sandbox-logs@1.0.0"};
    engine_props.set<std::vector<std::string>>({"engine", "libraries"}, libs);
    engine_props.set<std::vector<std::string>>({"engine", "sandbox"}, mods);

    // 2. Initialize engine (block scope to ensure it destroys correctly)
    {
        engine_t engine;
        engine.initialize(engine_props);

        flecs::world& world = engine.ecs;
        
        // 3. Test SDK logs
        logs::trace(world, "Test trace message");
        logs::debug(world, "Test debug message");
        logs::info(world, "Test info message");
        logs::warn(world, "Test warn message");
        logs::error(world, "Test error message");
    }

    // 4. Test that the file was flushed properly
    std::ifstream file("test_log_output.txt");
    REQUIRE(file.is_open());
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    
    REQUIRE(content.find("Test trace message") != std::string::npos);
    REQUIRE(content.find("Test debug message") != std::string::npos);
    REQUIRE(content.find("Test info message") != std::string::npos);
    REQUIRE(content.find("Test warn message") != std::string::npos);
    REQUIRE(content.find("Test error message") != std::string::npos);
}
