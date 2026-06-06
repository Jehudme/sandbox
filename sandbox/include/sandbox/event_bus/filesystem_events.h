#pragma once

#include <filesystem>
#include <vector>
#include <functional>
#include <string>
#include <stdexcept>

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
        mutable std::function<std::vector<std::byte>()> result_command;
    };

    struct write_request {
        std::filesystem::path virtual_path;
        mutable std::vector<std::byte> data;
        bool append_mode{false};
        mutable std::function<void()> result_command;
    };

    struct list_request {
        bool recursive{false};
        std::filesystem::path virtual_path;
        mutable std::function<std::vector<std::filesystem::path>()> result_command;
    };

    struct state_request {
        std::filesystem::path virtual_path;
        mutable std::function<file_metadata()> result_command;
    };

    struct absolute_request {
        std::filesystem::path virtual_path;
        mutable std::function<std::filesystem::path()> result_command;
    };

    struct delete_request {
        std::filesystem::path virtual_path;
        mutable std::function<void()> result_command;
    };

    struct mkdir_request {
        std::filesystem::path virtual_path;
        mutable std::function<void()> result_command;
    };

    struct rename_request {
        std::filesystem::path old_virtual_path;
        std::filesystem::path new_virtual_path;
        mutable std::function<void()> result_command;
    };

    struct copy_request {
        std::filesystem::path source_virtual_path;
        std::filesystem::path destination_virtual_path;
        mutable std::function<void()> result_command;
    };

    struct move_request {
        std::filesystem::path source_virtual_path;
        std::filesystem::path destination_virtual_path;
        mutable std::function<void()> result_command;
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

#include "sandbox/core/ecs.h"
#include "sandbox/event_bus/event_bus.h"

namespace sandbox::filesystem_controls {

    inline std::function<std::vector<std::byte>()> read(flecs::world ecs, const std::filesystem::path& path) {
        events::filesystem::read_request ev{ path };
        sandbox::events::publish(ecs, ev);
        return std::move(ev.result_command);
    }

    inline std::function<void()> write(flecs::world ecs, const std::filesystem::path& path, std::vector<std::byte>&& data, bool append = false) {
        events::filesystem::write_request ev{ path, std::move(data), append };
        sandbox::events::publish(ecs, ev);
        return std::move(ev.result_command);
    }

    inline std::function<std::vector<std::filesystem::path>()> list(flecs::world ecs, const std::filesystem::path& path, bool recursive = false) {
        events::filesystem::list_request ev{ recursive, path };
        sandbox::events::publish(ecs, ev);
        return std::move(ev.result_command);
    }

    inline std::function<void()> remove(flecs::world ecs, const std::filesystem::path& path) {
        events::filesystem::delete_request ev{ path };
        sandbox::events::publish(ecs, ev);
        return std::move(ev.result_command);
    }

    inline std::function<void()> mkdir(flecs::world ecs, const std::filesystem::path& path) {
        events::filesystem::mkdir_request ev{ path };
        sandbox::events::publish(ecs, ev);
        return std::move(ev.result_command);
    }

    inline std::function<void()> rename(flecs::world ecs, const std::filesystem::path& old_p, const std::filesystem::path& new_p) {
        events::filesystem::rename_request ev{ old_p, new_p };
        sandbox::events::publish(ecs, ev);
        return std::move(ev.result_command);
    }

    inline std::function<void()> copy(flecs::world ecs, const std::filesystem::path& src, const std::filesystem::path& dest) {
        events::filesystem::copy_request ev{ src, dest };
        sandbox::events::publish(ecs, ev);
        return std::move(ev.result_command);
    }

    inline std::function<void()> move(flecs::world ecs, const std::filesystem::path& src, const std::filesystem::path& dest) {
        events::filesystem::move_request ev{ src, dest };
        sandbox::events::publish(ecs, ev);
        return std::move(ev.result_command);
    }

    inline std::function<events::filesystem::file_metadata()> state(flecs::world ecs, const std::filesystem::path& path) {
        events::filesystem::state_request ev{ path };
        sandbox::events::publish(ecs, ev);
        return std::move(ev.result_command);
    }

    inline void mount(flecs::world ecs, const std::filesystem::path& phys, const std::filesystem::path& virt, bool read_only = true) {
        sandbox::events::publish(ecs, events::filesystem::mount_path{ virt, phys, read_only });
    }

    inline void unmount(flecs::world ecs, const std::filesystem::path& virt) {
        sandbox::events::publish(ecs, events::filesystem::unmount_path{ virt });
    }

    inline std::function<std::filesystem::path()> absolute(flecs::world ecs, const std::filesystem::path& path) {
        events::filesystem::absolute_request ev{ path };
        sandbox::events::publish(ecs, ev);
        return std::move(ev.result_command);
    }

} // namespace sandbox::filesystem_controls

// Returns clean closures to be executed at your convenience

#define SANDBOX_FS_FETCH_READ(world, path) \
    sandbox::filesystem_controls::read(world, path)

#define SANDBOX_FS_FETCH_WRITE(world, path, data, append) \
    sandbox::filesystem_controls::write(world, path, data, append)

#define SANDBOX_FS_FETCH_LIST(world, path, recursive) \
    sandbox::filesystem_controls::list(world, path, recursive)

#define SANDBOX_FS_FETCH_DELETE(world, path) \
    sandbox::filesystem_controls::remove(world, path)

#define SANDBOX_FS_FETCH_MKDIR(world, path) \
    sandbox::filesystem_controls::mkdir(world, path)

#define SANDBOX_FS_FETCH_RENAME(world, old_path, new_path) \
    sandbox::filesystem_controls::rename(world, old_path, new_path)

#define SANDBOX_FS_FETCH_COPY(world, src, dest) \
    sandbox::filesystem_controls::copy(world, src, dest)

#define SANDBOX_FS_FETCH_MOVE(world, src, dest) \
    sandbox::filesystem_controls::move(world, src, dest)

#define SANDBOX_FS_FETCH_STATE(world, path) \
    sandbox::filesystem_controls::state(world, path)

#define SANDBOX_FS_FETCH_ABSOLUTE(world, path) \
    sandbox::filesystem_controls::absolute(world, path)

// Evaluates inline immediately and returns values directly

#define SANDBOX_FS_EXEC_READ(world, path) \
    sandbox::filesystem_controls::read(world, path)()

#define SANDBOX_FS_EXEC_WRITE(world, path, data, append) \
    sandbox::filesystem_controls::write(world, path, data, append)()

#define SANDBOX_FS_EXEC_LIST(world, path, recursive) \
    sandbox::filesystem_controls::list(world, path, recursive)()

#define SANDBOX_FS_EXEC_DELETE(world, path) \
    sandbox::filesystem_controls::remove(world, path)()

#define SANDBOX_FS_EXEC_MKDIR(world, path) \
    sandbox::filesystem_controls::mkdir(world, path)()

#define SANDBOX_FS_EXEC_RENAME(world, old_path, new_path) \
    sandbox::filesystem_controls::rename(world, old_path, new_path)()

#define SANDBOX_FS_EXEC_COPY(world, src, dest) \
    sandbox::filesystem_controls::copy(world, src, dest)()

#define SANDBOX_FS_EXEC_MOVE(world, src, dest) \
    sandbox::filesystem_controls::move(world, src, dest)()

#define SANDBOX_FS_EXEC_STATE(world, path) \
    sandbox::filesystem_controls::state(world, path)()

#define SANDBOX_FS_EXEC_ABSOLUTE(world, path) \
    sandbox::filesystem_controls::absolute(world, path)()


#define SANDBOX_FS_MOUNT(world, physical_path, virtual_prefix, read_only) \
    sandbox::filesystem_controls::mount(world, physical_path, virtual_prefix, read_only)

#define SANDBOX_FS_UNMOUNT(world, virtual_prefix) \
    sandbox::filesystem_controls::unmount(world, virtual_prefix)
