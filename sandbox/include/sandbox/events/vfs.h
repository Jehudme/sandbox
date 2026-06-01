#pragma once

#include <filesystem>
#include <vector>
#include <functional>
#include <string>

namespace sandbox::events::vfs {

    // ============================================================================
    // File Status & Metadata Types
    // ============================================================================

    enum class file_type {
        regular,
        directory,
        symlink,
        unknown
    };

    struct file_metadata {
        std::filesystem::path virtual_path;
        int64_t size{0};                  // File size in bytes (0 for directories)
        int64_t creation_time{-1};        // Seconds since epoch (-1 if unsupported/unknown)
        int64_t modification_time{-1};    // Seconds since epoch (-1 if unsupported/unknown)
        int64_t access_time{-1};          // Seconds since epoch (-1 if unsupported/unknown)
        file_type type{file_type::unknown};
        bool read_only{false};
    };

    struct read_request {
        std::filesystem::path virtual_path;
        mutable std::function<std::vector<std::byte>()> result_command;
    };

    struct write_request {
        std::filesystem::path virtual_path;
        mutable std::vector<std::byte> data; // mutable allows moving data directly into the closure
        bool append_mode{false};
        mutable std::function<bool()> result_command;
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
        mutable std::function<bool()> result_command;
    };

    struct mkdir_request {
        std::filesystem::path virtual_path;
        mutable std::function<bool()> result_command;
    };

    struct rename_request {
        std::filesystem::path old_virtual_path;
        std::filesystem::path new_virtual_path;
        mutable std::function<bool()> result_command;
    };

    struct mount_path {
        std::filesystem::path virtual_prefix;
        std::filesystem::path physical_path;
        bool read_only{true};
    };

    struct unmount_path {
        std::filesystem::path virtual_prefix;
    };

} // namespace sandbox::events::vfs