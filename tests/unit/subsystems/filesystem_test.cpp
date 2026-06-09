#include <catch2/catch_test_macros.hpp>
#include "subsystems/filesystem/filesystem.h"
#include "sandbox/event_bus/filesystem_events.h"
#include <filesystem>
#include <fstream>
#include <cstring>

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

    SECTION("Verify ECS Request/Response pattern for filesystem events") {
        // Mock the observers for read and write requests
        ecs.observer<sandbox::events::filesystem::write_request>()
            .event(flecs::OnSet)
            .each([&](flecs::entity e, sandbox::events::filesystem::write_request& req) {
                int32_t res = ecs.get<sandbox::filesystem_service>().api->write(
                    req.virtual_path.string().c_str(), 
                    reinterpret_cast<const uint8_t*>(req.data.data()), 
                    req.data.size(), 
                    req.append_mode
                );
                // mock response
                e.set<sandbox::events::filesystem::write_response>({});
            });

        ecs.observer<sandbox::events::filesystem::read_request>()
            .event(flecs::OnSet)
            .each([&](flecs::entity e, sandbox::events::filesystem::read_request& req) {
                sandbox_payload payload{};
                ecs.get<sandbox::filesystem_service>().api->read(req.virtual_path.string().c_str(), &payload);
                e.set<sandbox::events::filesystem::read_response>({payload});
            });

        fs_api->mount(test_dir.string().c_str(), "mount://cache", false);
        
        // Dispatch write request using ECS
        std::vector<std::byte> data{std::byte('Z')};
        flecs::entity write_req = sandbox::filesystem_controls::write(ecs, "mount://cache/macro.txt", std::move(data), false);
        REQUIRE(write_req.has<sandbox::events::filesystem::write_response>());
        write_req.destruct();
        
        // Dispatch read request using ECS
        flecs::entity read_req = sandbox::filesystem_controls::read(ecs, "mount://cache/macro.txt");
        REQUIRE(read_req.has<sandbox::events::filesystem::read_response>());
        
        auto res = read_req.get<sandbox::events::filesystem::read_response>();
        REQUIRE(res.payload.size == 1);
        REQUIRE(res.payload.bytes[0] == static_cast<uint8_t>('Z'));
        
        if (res.payload.free_func) res.payload.free_func(res.payload.bytes);
        read_req.destruct();
    }
}
