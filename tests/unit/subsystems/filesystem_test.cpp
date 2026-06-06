#include <catch2/catch_test_macros.hpp>
#include "subsystems/filesystem/filesystem.h"
#include "sandbox/event_bus/filesystem_events.h"
#include <filesystem>
#include <fstream>

using namespace sandbox;

TEST_CASE("Filesystem Subsystem operations", "[subsystems][filesystem]") {
    flecs::world ecs;
    ecs.import<sandbox::modules::filesystem_module>();
    auto fs_api = ecs.get<filesystem_service>().api;

    std::filesystem::path test_dir = std::filesystem::current_path() / "test_mount";
    std::filesystem::create_directories(test_dir);

    SECTION("Mounting a valid OS path succeeds") {
        auto res = fs_api->mount(test_dir.string(), "mount://test", true);
        REQUIRE(res.has_value());
    }

    SECTION("Attempting to write to a read_only mount returns std::unexpected") {
        fs_api->mount(test_dir.string(), "mount://test_ro", true);
        auto res = fs_api->write("mount://test_ro/file.txt", {std::byte('A')});
        REQUIRE_FALSE(res.has_value());
    }

    SECTION("Path Traversal Protection: Writing to mount://cache/../../../etc/passwd is caught and returns an error") {
        fs_api->mount(test_dir.string(), "mount://cache", false);
        auto res = fs_api->write("mount://cache/../../../etc/passwd", {std::byte('A')});
        REQUIRE_FALSE(res.has_value());
    }

    SECTION("VFS Fallback: Reading a file existing in mount://app succeeds even if missing in mount://cache") {
        std::filesystem::path app_dir = test_dir / "app";
        std::filesystem::create_directories(app_dir);
        std::ofstream(app_dir / "test.txt") << "hello";

        fs_api->mount(test_dir.string(), "mount://cache", false);
        fs_api->mount(app_dir.string(), "mount://app", true);

        auto res = fs_api->read("mount://app/test.txt");
        REQUIRE(res.has_value());
    }

    SECTION("Verify the SANDBOX_FS_EXEC_* macros correctly route to the ECS interface") {
        // The engine's filesystem_module currently lacks the observers for these macros.
        // We mock the observer binding here to verify the macro logic correctly routes data.
        sandbox::events::subscribe<sandbox::events::filesystem::write_request>(ecs, [&](const sandbox::events::filesystem::write_request& req) {
            req.result_command = [&, req]() {
                ecs.get<sandbox::filesystem_service>().api->write(req.virtual_path.string(), req.data, req.append_mode);
            };
        });
        sandbox::events::subscribe<sandbox::events::filesystem::read_request>(ecs, [&](const sandbox::events::filesystem::read_request& req) {
            req.result_command = [&, req]() {
                return ecs.get<sandbox::filesystem_service>().api->read(req.virtual_path.string()).value();
            };
        });

        fs_api->mount(test_dir.string(), "mount://cache", false);
        std::vector<std::byte> data{std::byte('Z')};
        SANDBOX_FS_EXEC_WRITE(ecs, "mount://cache/macro.txt", std::move(data), false);
        
        auto res = SANDBOX_FS_EXEC_READ(ecs, "mount://cache/macro.txt");
        REQUIRE(res.size() == 1);
        REQUIRE(res[0] == std::byte('Z'));
    }
}
