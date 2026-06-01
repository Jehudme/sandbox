#pragma once

#include "sandbox/core/ecs.h"
#include "sandbox/events/vfs.h"
#include "sandbox/utilities/events.h"
#include <filesystem>
#include <vector>
#include <functional>
#include <string>

namespace sandbox::vfs_controls {

    inline std::function<std::vector<std::byte>()> read(flecs::world ecs, const std::filesystem::path& path) {
        events::vfs::read_request ev{ path };
        sandbox::events::publish(ecs, ev);
        return std::move(ev.result_command);
    }

    inline std::function<bool()> write(flecs::world ecs, const std::filesystem::path& path, std::vector<std::byte>&& data, bool append = false) {
        events::vfs::write_request ev{ path, std::move(data), append };
        sandbox::events::publish(ecs, ev);
        return std::move(ev.result_command);
    }

    inline std::function<std::vector<std::filesystem::path>()> list(flecs::world ecs, const std::filesystem::path& path, bool recursive = false) {
        events::vfs::list_request ev{ recursive, path };
        sandbox::events::publish(ecs, ev);
        return std::move(ev.result_command);
    }

    inline std::function<bool()> remove(flecs::world ecs, const std::filesystem::path& path) {
        events::vfs::delete_request ev{ path };
        sandbox::events::publish(ecs, ev);
        return std::move(ev.result_command);
    }

    inline std::function<bool()> mkdir(flecs::world ecs, const std::filesystem::path& path) {
        events::vfs::mkdir_request ev{ path };
        sandbox::events::publish(ecs, ev);
        return std::move(ev.result_command);
    }

    inline std::function<bool()> rename(flecs::world ecs, const std::filesystem::path& old_p, const std::filesystem::path& new_p) {
        events::vfs::rename_request ev{ old_p, new_p };
        sandbox::events::publish(ecs, ev);
        return std::move(ev.result_command);
    }

    inline std::function<events::vfs::file_metadata()> state(flecs::world ecs, const std::filesystem::path& path) {
        events::vfs::state_request ev{ path };
        sandbox::events::publish(ecs, ev);
        return std::move(ev.result_command);
    }

    inline void mount(flecs::world ecs, const std::filesystem::path& phys, const std::filesystem::path& virt, bool read_only = true) {
        sandbox::events::publish(ecs, events::vfs::mount_path{ virt, phys, read_only });
    }

    inline void unmount(flecs::world ecs, const std::filesystem::path& virt) {
        sandbox::events::publish(ecs, events::vfs::unmount_path{ virt });
    }

    inline std::function<std::filesystem::path()> absolute(flecs::world ecs, const std::filesystem::path& path) {
        events::vfs::absolute_request ev{ path };
        sandbox::events::publish(ecs, ev);
        return std::move(ev.result_command);
    }

} // namespace sandbox::vfs_controls

// ============================================================================
// 1. FETCH MACROS (Returns clean closures to be executed at your convenience)
// ============================================================================

#define SANDBOX_VFS_FETCH_READ(world, path) \
    sandbox::vfs_controls::read(world, path)

#define SANDBOX_VFS_FETCH_WRITE(world, path, data, append) \
    sandbox::vfs_controls::write(world, path, data, append)

#define SANDBOX_VFS_FETCH_LIST(world, path, recursive) \
    sandbox::vfs_controls::list(world, path, recursive)

#define SANDBOX_VFS_FETCH_DELETE(world, path) \
    sandbox::vfs_controls::remove(world, path)

#define SANDBOX_VFS_FETCH_MKDIR(world, path) \
    sandbox::vfs_controls::mkdir(world, path)

#define SANDBOX_VFS_FETCH_RENAME(world, old_path, new_path) \
    sandbox::vfs_controls::rename(world, old_path, new_path)

#define SANDBOX_VFS_FETCH_STATE(world, path) \
    sandbox::vfs_controls::state(world, path)

#define SANDBOX_VFS_FETCH_ABSOLUTE(world, path) \
    sandbox::vfs_controls::absolute(world, path)

// ============================================================================
// 2. EXECUTE MACROS (Evaluates inline immediately and returns values directly)
// ============================================================================

#define SANDBOX_VFS_EXEC_READ(world, path) \
    sandbox::vfs_controls::read(world, path)()

#define SANDBOX_VFS_EXEC_WRITE(world, path, data, append) \
    sandbox::vfs_controls::write(world, path, data, append)()

#define SANDBOX_VFS_EXEC_LIST(world, path, recursive) \
    sandbox::vfs_controls::list(world, path, recursive)()

#define SANDBOX_VFS_EXEC_DELETE(world, path) \
    sandbox::vfs_controls::remove(world, path)()

#define SANDBOX_VFS_EXEC_MKDIR(world, path) \
    sandbox::vfs_controls::mkdir(world, path)()

#define SANDBOX_VFS_EXEC_RENAME(world, old_path, new_path) \
    sandbox::vfs_controls::rename(world, old_path, new_path)()

#define SANDBOX_VFS_EXEC_STATE(world, path) \
    sandbox::vfs_controls::state(world, path)()

#define SANDBOX_VFS_EXEC_ABSOLUTE(world, path) \
    sandbox::vfs_controls::absolute(world, path)()

// ============================================================================
// 3. GLOBAL CONFIGURATION MACROS
// ============================================================================

#define SANDBOX_VFS_MOUNT(world, physical_path, virtual_prefix, read_only) \
    sandbox::vfs_controls::mount(world, physical_path, virtual_prefix, read_only)

#define SANDBOX_VFS_UNMOUNT(world, virtual_prefix) \
    sandbox::vfs_controls::unmount(world, virtual_prefix)

