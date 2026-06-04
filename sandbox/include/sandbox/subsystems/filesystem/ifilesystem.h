#pragma once

#include <filesystem>
#include <vector>
#include <string_view>
#include <string>
#include "sandbox/event_bus/filesystem_events.h"

namespace sandbox {

    class ifilesystem {
    public:
        virtual ~ifilesystem() = default;

        virtual void mount(std::string_view physical_path, std::string_view virtual_prefix, bool read_only = true) = 0;
        virtual void unmount(std::string_view virtual_prefix) = 0;
        virtual std::vector<std::byte> read(std::string_view virtual_path) = 0;
        virtual void write(std::string_view virtual_path, std::vector<std::byte> data, bool append = false) = 0;
        virtual std::vector<std::filesystem::path> list(std::string_view virtual_path, bool recursive = false) = 0;
        virtual void remove(std::string_view virtual_path) = 0;
        virtual void mkdir(std::string_view virtual_path) = 0;
        virtual void rename(std::string_view old_virtual_path, std::string_view new_virtual_path) = 0;
        virtual void copy(std::string_view source_virtual_path, std::string_view destination_virtual_path) = 0;
        virtual void move(std::string_view source_virtual_path, std::string_view destination_virtual_path) = 0;
        virtual events::filesystem::file_metadata state(std::string_view virtual_path) = 0;
        virtual std::filesystem::path absolute(std::string_view virtual_path) = 0;
    };

    struct filesystem_service {
        ifilesystem* api{nullptr};
    };

} // namespace sandbox
