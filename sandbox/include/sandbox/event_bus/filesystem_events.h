#pragma once

#include <stdexcept>
#include <string>
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
        std::string virtual_path;
        int64_t size{0};
        int64_t creation_time{-1};
        int64_t modification_time{-1};
        int64_t access_time{-1};
        file_type type{file_type::unknown};
        bool read_only{false};
    };

    struct fs_request {
        sandbox::abi::flatbuffer_payload payload;
    };
    
    struct fs_response {
        sandbox::abi::flatbuffer_payload payload;
    };

    class filesystem_error : public std::runtime_error {
    public:
        explicit filesystem_error(const std::string& message)
            : std::runtime_error(message) {}
    };

    class filesystem_mount_error : public filesystem_error {
    public:
        filesystem_mount_error(const std::string& context, const std::string& path, const std::string& details)
            : filesystem_error("[FS Mount Error] " + context + " failed for '" + path + "': " + details) {}
    };

    class filesystem_not_found_error : public filesystem_error {
    public:
        filesystem_not_found_error(const std::string& context, const std::string& path)
            : filesystem_error("[FS Not Found] " + context + " could not locate target: '" + path + "'") {}
    };

    class filesystem_read_error : public filesystem_error {
    public:
        filesystem_read_error(const std::string& context, const std::string& path, const std::string& details)
            : filesystem_error("[FS Read Error] " + context + " on '" + path + "': " + details) {}
    };

    class filesystem_write_error : public filesystem_error {
    public:
        filesystem_write_error(const std::string& context, const std::string& path, const std::string& details)
            : filesystem_error("[FS Write Error] " + context + " on '" + path + "': " + details) {}
    };

    class filesystem_system_error : public filesystem_error {
    public:
        filesystem_system_error(const std::string& context, const std::string& path, const std::string& details)
            : filesystem_error("[FS System Error] " + context + " on '" + path + "': " + details) {}
    };

} // namespace sandbox::events::filesystem
