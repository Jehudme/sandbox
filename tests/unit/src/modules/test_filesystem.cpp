#include <catch2/catch_all.hpp>
#include <flecs.h>
#include "core/engine.h"
#include <sandbox/sdk/configuration.hpp>
#include <sandbox/sdk/logs.hpp>
#include <sandbox/sdk/filesystem.hpp>
#include <sandbox/sdk/filesystem.hpp>
#include <sandbox/abi/bootstrapper.h>
#include "core/bootstrapper.h"
#include "core/exceptions.h"
#include <fstream>
#include <string>
#include <cstdlib>
#include <iostream>
#include <filesystem>

using namespace sandbox::core;

TEST_CASE("Filesystem Module: URI Mounting and Reading", "[filesystem][uri]") {
    bootstrapper_t::reset();

    // 1. Setup test physical files and zip
    std::filesystem::create_directories("test_physical_dir");
    std::ofstream dummy("test_physical_dir/test_file.txt");
    dummy << "Hello from physical directory!";
    dummy.close();

    std::filesystem::create_directories("test_zip_dir");
    std::ofstream zip_dummy("test_zip_dir/test_zip_file.txt");
    zip_dummy << "Hello from zip archive!";
    zip_dummy.close();
    
    // Create zip archive
    if (system("cd test_zip_dir && zip -q -r ../test_archive.zip *") != 0) {
        std::cerr << "Warning: Failed to create test zip archive." << std::endl;
    }

    // 2. Setup properties with mounts
    properties_t engine_props;
    engine_props.set<std::vector<std::string>>({"engine", "libraries"}, {"./sandbox_plugin.so"});
    engine_props.set<std::vector<std::string>>({"engine", "sandbox"}, {"sandbox-configuration@1.0.0", "sandbox-logs@1.0.0", "sandbox-filesystem@1.0.0", "sandbox-runtime@1.0.0"});
    
    // Add filesystem mount config
    engine_props.set<std::string>({"filesystem", "mounts", "app", "physical"}, "./test_archive.zip");
    engine_props.set<bool>({"filesystem", "mounts", "app", "readonly"}, true);

    engine_props.set<std::string>({"filesystem", "mounts", "cache", "physical"}, "./test_physical_dir");
    engine_props.set<bool>({"filesystem", "mounts", "cache", "readonly"}, false);

    // 3. Initialize engine
    engine_t engine;
    REQUIRE_NOTHROW(engine.initialize(engine_props));

    flecs::world& world = engine.entity_world;

    // 4. Test physical folder reading (cache://)
    SECTION("Read text from physical folder via cache://") {
        std::string content;
        REQUIRE_NOTHROW(content = sandbox::modules::filesystem::read_all_text(world, "cache://test_file.txt"));
        REQUIRE(content == "Hello from physical directory!");
    }

    // 5. Test zip archive reading (app://)
    SECTION("Read text from zip archive via app://") {
        std::string content;
        REQUIRE_NOTHROW(content = sandbox::modules::filesystem::read_all_text(world, "app://test_zip_file.txt"));
        REQUIRE(content == "Hello from zip archive!");
    }

    // 6. Test binary reading
    SECTION("Read bytes from physical folder") {
        std::vector<uint8_t> bytes;
        REQUIRE_NOTHROW(bytes = sandbox::modules::filesystem::read_all_bytes(world, "cache://test_file.txt"));
        REQUIRE(bytes.size() == 30);
    }

    // 7. Test exceptions
    SECTION("Throw on non-existent file in physical folder") {
        REQUIRE_THROWS_AS(sandbox::modules::filesystem::read_all_text(world, "cache://does_not_exist.txt"), std::runtime_error);
    }

    SECTION("Throw on non-existent file in zip archive") {
        REQUIRE_THROWS_AS(sandbox::modules::filesystem::read_all_text(world, "app://does_not_exist.txt"), std::runtime_error);
    }

    SECTION("Throw on unregistered URI scheme") {
        REQUIRE_THROWS_AS(sandbox::modules::filesystem::read_all_text(world, "unknown://test_file.txt"), std::runtime_error);
    }

    bootstrapper_t::reset();
}

TEST_CASE("Filesystem Module: Mutations (Create/Copy/Move/Directories)", "[filesystem][mutation]") {
    bootstrapper_t::reset();
    std::filesystem::remove_all("test_mutations_dir");
    std::filesystem::create_directories("test_mutations_dir");

    properties_t engine_props;
    engine_props.set<std::vector<std::string>>({"engine", "libraries"}, {"./sandbox_plugin.so"});
    engine_props.set<std::vector<std::string>>({"engine", "sandbox"}, {"sandbox-configuration@1.0.0", "sandbox-logs@1.0.0", "sandbox-filesystem@1.0.0", "sandbox-runtime@1.0.0"});
    
    engine_props.set<std::string>({"filesystem", "mounts", "test", "physical"}, "./test_mutations_dir");
    engine_props.set<bool>({"filesystem", "mounts", "test", "readonly"}, false);

    engine_t engine;
    REQUIRE_NOTHROW(engine.initialize(engine_props));
    flecs::world& world = engine.entity_world;

    SECTION("Create and remove file") {
        REQUIRE(sandbox::modules::filesystem::create_file(world, "test://new_file.txt", false));
        REQUIRE(std::filesystem::exists("test_mutations_dir/new_file.txt"));
        REQUIRE(sandbox::modules::filesystem::remove_file(world, "test://new_file.txt"));
        REQUIRE(!std::filesystem::exists("test_mutations_dir/new_file.txt"));
    }

    SECTION("Create directory and file with force_path") {
        REQUIRE(sandbox::modules::filesystem::create_directory(world, "test://dir1", false));
        REQUIRE(std::filesystem::exists("test_mutations_dir/dir1"));
        
        REQUIRE(sandbox::modules::filesystem::create_file(world, "test://dir2/file.txt", true));
        REQUIRE(std::filesystem::exists("test_mutations_dir/dir2/file.txt"));
        
        REQUIRE(sandbox::modules::filesystem::remove_directory(world, "test://dir1"));
        REQUIRE(sandbox::modules::filesystem::remove_directory(world, "test://dir2"));
    }

    SECTION("Copy and move files") {
        REQUIRE(sandbox::modules::filesystem::create_file(world, "test://src.txt", false));
        REQUIRE(sandbox::modules::filesystem::write_all(world, "test://src.txt", "hello", 5, false));
        
        REQUIRE(sandbox::modules::filesystem::copy(world, "test://src.txt", "test://copied.txt", false, false));
        REQUIRE(std::filesystem::exists("test_mutations_dir/copied.txt"));
        
        REQUIRE(sandbox::modules::filesystem::move(world, "test://copied.txt", "test://moved.txt", false, false));
        REQUIRE(!std::filesystem::exists("test_mutations_dir/copied.txt"));
        REQUIRE(std::filesystem::exists("test_mutations_dir/moved.txt"));
        
        REQUIRE(sandbox::modules::filesystem::remove_file(world, "test://src.txt"));
        REQUIRE(sandbox::modules::filesystem::remove_file(world, "test://moved.txt"));
    }

    bootstrapper_t::reset();
}
