#include <catch2/catch_test_macros.hpp>
#include "subsystems/filesystem/filesystem.h"

#include "sandbox/generated/schemas/filesystem_generated.h"
#include <filesystem>
#include <fstream>
#include <cstring>
#include <iostream>

using namespace sandbox;

TEST_CASE("Filesystem Subsystem operations", "[subsystems][filesystem]") {
    flecs::world ecs;
    ecs.import<sandbox::modules::filesystem_module>();
    auto fs_api = ecs.get<filesystem_service>().api;

    std::filesystem::path test_dir = std::filesystem::current_path() / "test_mount";
    std::filesystem::create_directories(test_dir);

    SECTION("Mounting a valid OS path succeeds") {
        auto res = fs_api->mount(test_dir.string().c_str(), "mount://test", true);
        REQUIRE(res == 0);
    }

    SECTION("Attempting to write to a read_only mount returns error") {
        fs_api->mount(test_dir.string().c_str(), "mount://test_ro", true);
        uint8_t dummy = 'A';
        auto res = fs_api->write("mount://test_ro/file.txt", &dummy, 1, false);
        REQUIRE(res != 0);
    }

    SECTION("Path Traversal Protection: Writing to mount://cache/../../../etc/passwd is caught and returns an error") {
        fs_api->mount(test_dir.string().c_str(), "mount://cache", false);
        uint8_t dummy = 'A';
        auto res = fs_api->write("mount://cache/../../../etc/passwd", &dummy, 1, false);
        REQUIRE(res != 0);
    }

    SECTION("VFS Fallback: Reading a file existing in mount://app succeeds even if missing in mount://cache") {
        std::filesystem::path app_dir = test_dir / "app";
        std::filesystem::create_directories(app_dir);
        std::ofstream(app_dir / "test.txt") << "hello";

        fs_api->mount(test_dir.string().c_str(), "mount://cache", false);
        fs_api->mount(app_dir.string().c_str(), "mount://app", true);

        sandbox_payload payload{};
        auto res = fs_api->read("mount://app/test.txt", &payload);
        REQUIRE(res == 0);
        if (payload.free_func) payload.free_func(payload.bytes);
    }



    SECTION("Raw C-ABI methods return correct FlatBuffer payloads") {
        fs_api->mount(test_dir.string().c_str(), "mount://cache", false);

        // write a file using raw API
        uint8_t data[] = { 'Z', 'Z' };
        REQUIRE(fs_api->write("mount://cache/macro.txt", data, 2, false) == 0);

        // state query using raw API
        sandbox_payload state_payload{};
        REQUIRE(fs_api->state("mount://cache/macro.txt", &state_payload) == 0);
        auto meta = flatbuffers::GetRoot<sandbox::schemas::FileMetadata>(state_payload.bytes);
        REQUIRE(meta != nullptr);
        REQUIRE(meta->size() == 2);
        REQUIRE(meta->type() == sandbox::schemas::FileType_Regular);
        if (state_payload.free_func) state_payload.free_func(state_payload.bytes);

        // list files using raw API
        sandbox_payload list_payload{};
        REQUIRE(fs_api->list("mount://cache", false, &list_payload) == 0);
        auto str_list = flatbuffers::GetRoot<sandbox::schemas::StringList>(list_payload.bytes);
        REQUIRE(str_list != nullptr);
        REQUIRE(str_list->items() != nullptr);
        REQUIRE(str_list->items()->size() >= 1);
        bool found = false;
        for (size_t i = 0; i < str_list->items()->size(); ++i) {
            std::string path = str_list->items()->Get(i)->c_str();
            if (path.length() >= 9 && path.substr(path.length() - 9) == "macro.txt") {
                found = true; break;
            }
        }
        REQUIRE(found);
        if (list_payload.free_func) list_payload.free_func(list_payload.bytes);
    }
}
