#include <catch2/catch_all.hpp>
#include <flecs.h>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "sandbox/abi/bootstrapper.h"
#include "sandbox/sdk/engine.hpp"
#include <sandbox/sdk/filesystem.hpp>
#include "sandbox/sdk/properties.hpp"
#include <sandbox/sdk/configuration.hpp>
#include "../../test_accessor.h"

namespace fs = std::filesystem;

TEST_CASE("Application Module: Initialization and Orchestration", "[application][module]") {
    // 1. Create a temporary app directory with a configuration and a dummy plugin
    fs::path temp_app_dir = fs::temp_directory_path() / "sandbox_test_app";
    fs::create_directories(temp_app_dir);

    // configuration.json
    fs::path config_path = temp_app_dir / "configuration.json";
    std::ofstream config_file(config_path);
    config_file << R"({
        "test": {
            "value": 42
        }
    })";
    config_file.close();

    // plugins/ directory (we won't put a real native library to avoid crash, but test directory parsing)
    fs::path plugins_dir = temp_app_dir / "plugins";
    fs::create_directories(plugins_dir);

    SECTION("Initializes correctly and ingests configuration") {
        sandbox::engine e;

        // Create properties simulating CLI parser
        sandbox::properties props;
        props.set("filesystem/mounts/app/physical", temp_app_dir.string());
        props.set("filesystem/mounts/app/readonly", true);

        std::vector<std::string> library_paths = {"./cmake-build-debug/bin/sandbox_plugin.so"};
        props.set_array("booting-configuration/libraries", library_paths);

        // Add modules
        std::vector<std::string> modules = {
            "sandbox-configuration@1.0.0", 
            "sandbox-logs@1.0.0", 
            "sandbox-filesystem@1.0.0", 
            "sandbox-runtime@1.0.0",
            "sandbox-application@1.0.0"
        };
        props.set_array("booting-configuration/modules", modules);

        // Initialize engine (this will boot all modules including application)
        REQUIRE(e.initialize(props) == true);

        // Verify configuration was ingested
        flecs::world ecs(static_cast<ecs_world_t*>(e.get_ecs()));
        auto config = sandbox::modules::configuration::get_properties(ecs);
        REQUIRE(config.is_valid());

        auto test_val = config.get<int>("test/value");
        REQUIRE(test_val.has_value());
        REQUIRE(test_val.value() == 42);

        // Verify cache and save mounts were created
        REQUIRE(sandbox::modules::filesystem::exists(ecs, "cache://"));
        REQUIRE(sandbox::modules::filesystem::exists(ecs, "save://"));
    }

    // Cleanup
    fs::remove_all(temp_app_dir);
}
