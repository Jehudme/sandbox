#pragma once

#include <stdexcept>
#include <string>
#include <filesystem>

namespace sandbox::events::vfs {

    class vfs_error : public std::runtime_error {
    public:
        explicit vfs_error(const std::string& message);
    };

    class vfs_mount_error : public vfs_error {
    public:
        vfs_mount_error(const std::string& context, const std::filesystem::path& path, const std::string& details);
    };

    class vfs_not_found_error : public vfs_error {
    public:
        vfs_not_found_error(const std::string& context, const std::filesystem::path& path);
    };

    class vfs_read_error : public vfs_error {
    public:
        vfs_read_error(const std::string& context, const std::filesystem::path& path, const std::string& details);
    };

    class vfs_write_error : public vfs_error {
    public:
        vfs_write_error(const std::string& context, const std::filesystem::path& path, const std::string& details);
    };

    class vfs_system_error : public vfs_error {
    public:
        vfs_system_error(const std::string& context, const std::filesystem::path& path, const std::string& details);
    };

}