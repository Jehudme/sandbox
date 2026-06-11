#include <catch2/catch_test_macros.hpp>
#include "modules/filesystem/filesystem.h"
#include <sandbox/modules/filesystem/filesystem_api.h>

#include <filesystem>
#include <fstream>
#include <cstring>
#include <iostream>

using namespace sandbox;

TEST_CASE("Filesystem Subsystem operations", "[subsystems][filesystem]") {
    flecs::world ecs;
    ecs.import<sandbox::modules::filesystem_module>();
    auto fs_api = sandbox::sdk::filesystem(ecs);

    std::filesystem::path test_dir = std::filesystem::current_path() / "test_mount";
    std::filesystem::create_directories(test_dir);

    SECTION("Mounting a valid OS path succeeds") {
        auto res = fs_api.mount(test_dir.string(), "mount://test", true);
        REQUIRE(res.has_value());
    }

    SECTION("Attempting to write to a read_only mount returns error") {
        fs_api.mount(test_dir.string(), "mount://test_ro", true);
        std::vector<std::byte> dummy = { std::byte{'A'} };
        auto res = fs_api.write("mount://test_ro/file.txt", dummy, false);
        REQUIRE(!res.has_value());
    }

    SECTION("Path Traversal Protection: Writing to mount://cache/../../../etc/passwd is caught and returns an error") {
        fs_api.mount(test_dir.string(), "mount://cache", false);
        std::vector<std::byte> dummy = { std::byte{'A'} };
        auto res = fs_api.write("mount://cache/../../../etc/passwd", dummy, false);
        REQUIRE(!res.has_value());
    }

    SECTION("VFS Fallback: Reading a file existing in mount://app succeeds even if missing in mount://cache") {
        std::filesystem::path app_dir = test_dir / "app";
        std::filesystem::create_directories(app_dir);
        std::ofstream(app_dir / "test.txt") << "hello";

        fs_api.mount(test_dir.string(), "mount://cache", false);
        fs_api.mount(app_dir.string(), "mount://app", true);

        auto res = fs_api.read_text("mount://app/test.txt");
        REQUIRE(res.has_value());
    }

    SECTION("Raw C-ABI methods return correct FlatBuffer payloads") {
        fs_api.mount(test_dir.string(), "mount://cache", false);

        // write a file using raw API
        std::vector<std::byte> data = { std::byte{'Z'}, std::byte{'Z'} };
        REQUIRE(fs_api.write("mount://cache/macro.txt", data, false).has_value());

        // state query using raw API
        auto state_res = fs_api.state("mount://cache/macro.txt");
        REQUIRE(state_res.has_value());
        REQUIRE(state_res->size == 2);
        REQUIRE(state_res->is_directory == false);

        // list files using raw API
        auto list_res = fs_api.list("mount://cache", false);
        REQUIRE(list_res.has_value());
        REQUIRE(list_res->size() >= 1);
        bool found = false;
        for (const auto& pstr : *list_res) {
            if (pstr.length() >= 9 && pstr.substr(pstr.length() - 9) == "macro.txt") {
                found = true; break;
            }
        }
        REQUIRE(found);
    }
}

