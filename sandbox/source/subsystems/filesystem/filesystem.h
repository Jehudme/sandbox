#pragma once

#include "sandbox/core/ecs.h"
#include "sandbox/event_bus/filesystem_events.h"
#include "sandbox/subsystems/filesystem/ifilesystem.h"
#include <expected>
#include <string>
#include <string_view>
#include <filesystem>
#include <unordered_map>

namespace sandbox::modules {

    class filesystem_module : public ifilesystem {
    public:
        filesystem_module(world& ecs);
        ~filesystem_module() override;

        std::expected<void, std::string> mount(std::string_view physical_path, std::string_view virtual_prefix, bool read_only = true) override;
        std::expected<void, std::string> unmount(std::string_view virtual_prefix) override;
        std::expected<std::vector<std::byte>, std::string> read(std::string_view virtual_path) const override;
        std::expected<void, std::string> write(std::string_view virtual_path, std::vector<std::byte> data, bool append = false) override;
        std::expected<std::vector<std::filesystem::path>, std::string> list(std::string_view virtual_path, bool recursive = false) const override;
        std::expected<void, std::string> remove(std::string_view virtual_path) override;
        std::expected<void, std::string> mkdir(std::string_view virtual_path) override;
        std::expected<void, std::string> rename(std::string_view old_virtual_path, std::string_view new_virtual_path) override;
        std::expected<void, std::string> copy(std::string_view source_virtual_path, std::string_view destination_virtual_path) override;
        std::expected<void, std::string> move(std::string_view source_virtual_path, std::string_view destination_virtual_path) override;
        std::expected<events::filesystem::file_metadata, std::string> state(std::string_view virtual_path) const override;
        std::expected<std::filesystem::path, std::string> absolute(std::string_view virtual_path) const override;

    private:
        std::string get_mount_prefix(std::string_view v_path) const;
        std::string get_sub_path(std::string_view v_path) const;
        std::string get_physfs_path(std::string_view v_path) const;

        std::string get_physfs_error(const std::string& context, const std::filesystem::path& path) const;

        std::expected<std::filesystem::path, std::string> resolve_physical_write_path(const std::filesystem::path& virtual_path) const;

        std::unordered_map<std::string, std::filesystem::path> m_writable_mounts;
    };

} // namespace sandbox::modules
