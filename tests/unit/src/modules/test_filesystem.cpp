#include <catch2/catch_all.hpp>
#include <flecs.h>
#include "core/engine.h"
#include <sandbox/sdk/configuration.hpp>
#include <sandbox/sdk/logs.hpp>
#include <sandbox/sdk/filesystem.hpp>
#include <sandbox/sdk/filesystem.hpp>
#include <sandbox/abi/bootstrapper.h>
#include "../../test_accessor.h"
#include "core/exceptions.h"
#include <fstream>
#include <string>
#include <cstdlib>
#include <iostream>
#include <filesystem>

using namespace sandbox::core;

TEST_CASE("Filesystem Module: URI Mounting and Reading", "[filesystem][uri]") {

    // 1. Setup test physical files and zip
    std::filesystem::create_directories("cache");
    std::ofstream dummy("cache/test_file.txt");
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
    engine_props.set<std::vector<std::string>>({"booting-configuration", "libraries"}, {"./cmake-build-debug/bin/sandbox_plugin.so"});
    engine_props.set<std::vector<std::string>>({"booting-configuration", "modules"}, {"sandbox-configuration@1.0.0", "sandbox-logs@1.0.0", "sandbox-filesystem@1.0.0", "sandbox-runtime@1.0.0"});
    
    // Add filesystem mount config
    engine_props.set<std::string>({"booting-configuration", "mount-path"}, "./test_archive.zip");

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

}

TEST_CASE("Filesystem Module: Mutations (Create/Copy/Move/Directories)", "[filesystem][mutation]") {
    std::filesystem::remove_all("test_mutations_dir");
    std::filesystem::create_directories("test_mutations_dir");

    properties_t engine_props;
    engine_props.set<std::vector<std::string>>({"booting-configuration", "libraries"}, {"./cmake-build-debug/bin/sandbox_plugin.so"});
    engine_props.set<std::vector<std::string>>({"booting-configuration", "modules"}, {"sandbox-configuration@1.0.0", "sandbox-logs@1.0.0", "sandbox-filesystem@1.0.0", "sandbox-runtime@1.0.0"});
    
    engine_props.set<std::string>({"booting-configuration", "mount-path"}, "./test_mutations_dir");

    engine_t engine;
    REQUIRE_NOTHROW(engine.initialize(engine_props));
    flecs::world& world = engine.entity_world;

    SECTION("Create and remove file") {
        REQUIRE(sandbox::modules::filesystem::create_file(world, "cache://new_file.txt", false));
        REQUIRE(std::filesystem::exists("test_mutations_dir/cache/new_file.txt"));
        REQUIRE(sandbox::modules::filesystem::remove_file(world, "cache://new_file.txt"));
        REQUIRE(!std::filesystem::exists("test_mutations_dir/cache/new_file.txt"));
    }

    SECTION("Create directory and file with force_path") {
        REQUIRE(sandbox::modules::filesystem::create_directory(world, "cache://dir1", false));
        REQUIRE(std::filesystem::exists("test_mutations_dir/cache/dir1"));
        
        REQUIRE(sandbox::modules::filesystem::create_file(world, "cache://dir2/file.txt", true));
        REQUIRE(std::filesystem::exists("test_mutations_dir/cache/dir2/file.txt"));
        
        REQUIRE(sandbox::modules::filesystem::remove_directory(world, "cache://dir1"));
        REQUIRE(sandbox::modules::filesystem::remove_directory(world, "cache://dir2"));
    }

    SECTION("Copy and move files") {
        REQUIRE(sandbox::modules::filesystem::create_file(world, "cache://src.txt", false));
        REQUIRE(sandbox::modules::filesystem::write_all(world, "cache://src.txt", "hello", 5, false));
        
        REQUIRE(sandbox::modules::filesystem::copy(world, "cache://src.txt", "cache://copied.txt", false, false));
        REQUIRE(std::filesystem::exists("test_mutations_dir/cache/copied.txt"));
        
        REQUIRE(sandbox::modules::filesystem::move(world, "cache://copied.txt", "cache://moved.txt", false, false));
        REQUIRE(!std::filesystem::exists("test_mutations_dir/cache/copied.txt"));
        REQUIRE(std::filesystem::exists("test_mutations_dir/cache/moved.txt"));
        
        REQUIRE(sandbox::modules::filesystem::remove_file(world, "cache://src.txt"));
        REQUIRE(sandbox::modules::filesystem::remove_file(world, "cache://moved.txt"));
    }

}
