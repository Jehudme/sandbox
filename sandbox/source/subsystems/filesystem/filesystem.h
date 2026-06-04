#pragma once

#include "sandbox/core/ecs.h"
#include "sandbox/event_bus/filesystem_events.h"
#include "sandbox/subsystems/filesystem/ifilesystem.h"
#include <string>
#include <string_view>
#include <filesystem>
#include <unordered_map>

namespace sandbox::modules {

    class filesystem_module : public ifilesystem {
    public:
        filesystem_module(world& ecs);
        ~filesystem_module() override;

        void mount(std::string_view physical_path, std::string_view virtual_prefix, bool read_only = true) override;
        void unmount(std::string_view virtual_prefix) override;
        std::vector<std::byte> read(std::string_view virtual_path) override;
        void write(std::string_view virtual_path, std::vector<std::byte> data, bool append = false) override;
        std::vector<std::filesystem::path> list(std::string_view virtual_path, bool recursive = false) override;
        void remove(std::string_view virtual_path) override;
        void mkdir(std::string_view virtual_path) override;
        void rename(std::string_view old_virtual_path, std::string_view new_virtual_path) override;
        void copy(std::string_view source_virtual_path, std::string_view destination_virtual_path) override;
        void move(std::string_view source_virtual_path, std::string_view destination_virtual_path) override;
        events::filesystem::file_metadata state(std::string_view virtual_path) override;
        std::filesystem::path absolute(std::string_view virtual_path) override;

    private:
        std::string get_mount_prefix(std::string_view v_path) const;
        std::string get_sub_path(std::string_view v_path) const;
        std::string get_physfs_path(std::string_view v_path) const;

        void throw_physfs_error(const std::string& context, const std::filesystem::path& path) const;

        std::filesystem::path resolve_physical_write_path(const std::filesystem::path& virtual_path) const;

        std::unordered_map<std::string, std::filesystem::path> m_writable_mounts;
    };

} // namespace sandbox::modules
