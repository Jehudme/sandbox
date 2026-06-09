#pragma once

#include <filesystem>
#include <vector>
#include <string>
#include <stdexcept>
#include "sandbox/core/abi_types.h"
#include "sandbox/core/ecs.h"

namespace sandbox::events::filesystem {

    enum class file_type {
        regular,
        directory,
        symlink,
        unknown
    };

    struct file_metadata {
        std::filesystem::path virtual_path;
        int64_t size{0};
        int64_t creation_time{-1};
        int64_t modification_time{-1};
        int64_t access_time{-1};
        file_type type{file_type::unknown};
        bool read_only{false};
    };

    struct read_request {
        std::filesystem::path virtual_path;
    };
    struct read_response {
        sandbox_payload payload;
    };

    struct write_request {
        std::filesystem::path virtual_path;
        std::vector<std::byte> data;
        bool append_mode{false};
    };
    struct write_response {
        sandbox_payload payload;
    };

    struct list_request {
        bool recursive{false};
        std::filesystem::path virtual_path;
    };
    struct list_response {
        sandbox_payload payload;
    };

    struct state_request {
        std::filesystem::path virtual_path;
    };
    struct state_response {
        sandbox_payload payload;
    };

    struct absolute_request {
        std::filesystem::path virtual_path;
    };
    struct absolute_response {
        sandbox_payload payload;
    };

    struct delete_request {
        std::filesystem::path virtual_path;
    };
    struct delete_response {
        sandbox_payload payload;
    };

    struct mkdir_request {
        std::filesystem::path virtual_path;
    };
    struct mkdir_response {
        sandbox_payload payload;
    };

    struct rename_request {
        std::filesystem::path old_virtual_path;
        std::filesystem::path new_virtual_path;
    };
    struct rename_response {
        sandbox_payload payload;
    };

    struct copy_request {
        std::filesystem::path source_virtual_path;
        std::filesystem::path destination_virtual_path;
    };
    struct copy_response {
        sandbox_payload payload;
    };

    struct move_request {
        std::filesystem::path source_virtual_path;
        std::filesystem::path destination_virtual_path;
    };
    struct move_response {
        sandbox_payload payload;
    };

    struct mount_path {
        std::filesystem::path virtual_prefix;
        std::filesystem::path physical_path;
        bool read_only{true};
    };

    struct unmount_path {
        std::filesystem::path virtual_prefix;
    };

    class filesystem_error : public std::runtime_error {
    public:
        explicit filesystem_error(const std::string& message)
            : std::runtime_error(message) {}
    };

    class filesystem_mount_error : public filesystem_error {
    public:
        filesystem_mount_error(const std::string& context, const std::filesystem::path& path, const std::string& details)
            : filesystem_error("[FS Mount Error] " + context + " failed for '" + path.string() + "': " + details) {}
    };

    class filesystem_not_found_error : public filesystem_error {
    public:
        filesystem_not_found_error(const std::string& context, const std::filesystem::path& path)
            : filesystem_error("[FS Not Found] " + context + " could not locate target: '" + path.string() + "'") {}
    };

    class filesystem_read_error : public filesystem_error {
    public:
        filesystem_read_error(const std::string& context, const std::filesystem::path& path, const std::string& details)
            : filesystem_error("[FS Read Error] " + context + " on '" + path.string() + "': " + details) {}
    };

    class filesystem_write_error : public filesystem_error {
    public:
        filesystem_write_error(const std::string& context, const std::filesystem::path& path, const std::string& details)
            : filesystem_error("[FS Write Error] " + context + " on '" + path.string() + "': " + details) {}
    };

    class filesystem_system_error : public filesystem_error {
    public:
        filesystem_system_error(const std::string& context, const std::filesystem::path& path, const std::string& details)
            : filesystem_error("[FS System Error] " + context + " on '" + path.string() + "': " + details) {}
    };

} // namespace sandbox::events::filesystem

namespace sandbox::filesystem_controls {

    inline flecs::entity read(flecs::world ecs, const std::filesystem::path& path) {
        return ecs.entity().set<events::filesystem::read_request>({ path });
    }

    inline flecs::entity write(flecs::world ecs, const std::filesystem::path& path, std::vector<std::byte>&& data, bool append = false) {
        return ecs.entity().set<events::filesystem::write_request>({ path, std::move(data), append });
    }

    inline flecs::entity list(flecs::world ecs, const std::filesystem::path& path, bool recursive = false) {
        return ecs.entity().set<events::filesystem::list_request>({ recursive, path });
    }

    inline flecs::entity remove(flecs::world ecs, const std::filesystem::path& path) {
        return ecs.entity().set<events::filesystem::delete_request>({ path });
    }

    inline flecs::entity mkdir(flecs::world ecs, const std::filesystem::path& path) {
        return ecs.entity().set<events::filesystem::mkdir_request>({ path });
    }

    inline flecs::entity rename(flecs::world ecs, const std::filesystem::path& old_p, const std::filesystem::path& new_p) {
        return ecs.entity().set<events::filesystem::rename_request>({ old_p, new_p });
    }

    inline flecs::entity copy(flecs::world ecs, const std::filesystem::path& src, const std::filesystem::path& dest) {
        return ecs.entity().set<events::filesystem::copy_request>({ src, dest });
    }

    inline flecs::entity move(flecs::world ecs, const std::filesystem::path& src, const std::filesystem::path& dest) {
        return ecs.entity().set<events::filesystem::move_request>({ src, dest });
    }

    inline flecs::entity state(flecs::world ecs, const std::filesystem::path& path) {
        return ecs.entity().set<events::filesystem::state_request>({ path });
    }

    inline flecs::entity mount(flecs::world ecs, const std::filesystem::path& phys, const std::filesystem::path& virt, bool read_only = true) {
        return ecs.entity().set<events::filesystem::mount_path>({ virt, phys, read_only });
    }

    inline flecs::entity unmount(flecs::world ecs, const std::filesystem::path& virt) {
        return ecs.entity().set<events::filesystem::unmount_path>({ virt });
    }

    inline flecs::entity absolute(flecs::world ecs, const std::filesystem::path& path) {
        return ecs.entity().set<events::filesystem::absolute_request>({ path });
    }

} // namespace sandbox::filesystem_controls
