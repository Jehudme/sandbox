#include "sandbox/exceptions/vfs_exceptions.h"

namespace sandbox::events::vfs {

    vfs_error::vfs_error(const std::string& message)
        : std::runtime_error(message) {}

    vfs_mount_error::vfs_mount_error(const std::string& context, const std::filesystem::path& path, const std::string& details)
        : vfs_error("[VFS Mount Error] " + context + " failed for '" + path.string() + "': " + details) {}

    vfs_not_found_error::vfs_not_found_error(const std::string& context, const std::filesystem::path& path)
        : vfs_error("[VFS Not Found] " + context + " could not locate target: '" + path.string() + "'") {}

    vfs_read_error::vfs_read_error(const std::string& context, const std::filesystem::path& path, const std::string& details)
        : vfs_error("[VFS Read Error] " + context + " on '" + path.string() + "': " + details) {}

    vfs_write_error::vfs_write_error(const std::string& context, const std::filesystem::path& path, const std::string& details)
        : vfs_error("[VFS Write Error] " + context + " on '" + path.string() + "': " + details) {}

    vfs_system_error::vfs_system_error(const std::string& context, const std::filesystem::path& path, const std::string& details)
        : vfs_error("[VFS System Error] " + context + " on '" + path.string() + "': " + details) {}

}