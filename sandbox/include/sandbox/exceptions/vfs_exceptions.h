#pragma once

#include <stdexcept>
#include <string>
#include <filesystem>
#include "sandbox/core/platform.h"

namespace sandbox::events::vfs {

    class SANDBOX_API vfs_error : public std::runtime_error {
    public:
        explicit vfs_error(const std::string& message);
    };

    class SANDBOX_API vfs_mount_error : public vfs_error {
    public:
        vfs_mount_error(const std::string& context, const std::filesystem::path& path, const std::string& details);
    };

    class SANDBOX_API vfs_not_found_error : public vfs_error {
    public:
        vfs_not_found_error(const std::string& context, const std::filesystem::path& path);
    };

    class SANDBOX_API vfs_read_error : public vfs_error {
    public:
        vfs_read_error(const std::string& context, const std::filesystem::path& path, const std::string& details);
    };

    class SANDBOX_API vfs_write_error : public vfs_error {
    public:
        vfs_write_error(const std::string& context, const std::filesystem::path& path, const std::string& details);
    };

    class SANDBOX_API vfs_system_error : public vfs_error {
    public:
        vfs_system_error(const std::string& context, const std::filesystem::path& path, const std::string& details);
    };

}