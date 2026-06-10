#pragma once

#include "sandbox/core/ecs.h"
#include "generated/schemas/filesystem_generated.h"
#include "subsystems/filesystem/ifilesystem.h"
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

        int32_t mount(const char* physical_path, const char* virtual_prefix, bool read_only) override;
        int32_t unmount(const char* virtual_prefix) override;
        int32_t read(const char* virtual_path, sandbox_payload* out_payload) const override;
        int32_t write(const char* virtual_path, const uint8_t* data, size_t size, bool append) override;
        int32_t list(const char* virtual_path, bool recursive, sandbox_payload* out_payload) const override;
        int32_t remove(const char* virtual_path) override;
        int32_t mkdir(const char* virtual_path) override;
        int32_t rename(const char* old_virtual_path, const char* new_virtual_path) override;
        int32_t copy(const char* source_virtual_path, const char* destination_virtual_path) override;
        int32_t move(const char* source_virtual_path, const char* destination_virtual_path) override;
        [[nodiscard]] int32_t state(const char* virtual_path, sandbox_payload* out_payload) const override;
        [[nodiscard]] int32_t absolute(const char* virtual_path, sandbox_payload* out_payload) const override;

        std::expected<void, std::string> mount_impl(std::string_view physical_path, std::string_view virtual_prefix, bool read_only = true);
        std::expected<void, std::string> unmount_impl(std::string_view virtual_prefix);
        std::expected<std::vector<std::byte>, std::string> read_impl(std::string_view virtual_path) const;
        std::expected<void, std::string> write_impl(std::string_view virtual_path, std::vector<std::byte> data, bool append = false);
        std::expected<std::vector<std::filesystem::path>, std::string> list_impl(std::string_view virtual_path, bool recursive = false) const;
        std::expected<void, std::string> remove_impl(std::string_view virtual_path);
        std::expected<void, std::string> mkdir_impl(std::string_view virtual_path);
        std::expected<void, std::string> rename_impl(std::string_view old_virtual_path, std::string_view new_virtual_path);
        std::expected<void, std::string> copy_impl(std::string_view source_virtual_path, std::string_view destination_virtual_path);
        std::expected<void, std::string> move_impl(std::string_view source_virtual_path, std::string_view destination_virtual_path);
        [[nodiscard]] std::expected<sandbox::schemas::FileMetadataT, std::string> state_impl(std::string_view virtual_path) const;
        [[nodiscard]] std::expected<std::filesystem::path, std::string> absolute_impl(std::string_view virtual_path) const;

        void set_property(const char* key, const char* json_value) override;
        int32_t get_property(const char* key, sandbox_payload* out_payload) const override;

    private:
        std::string get_mount_prefix(std::string_view v_path) const;
        std::string get_sub_path(std::string_view v_path) const;
        std::string get_physfs_path(std::string_view v_path) const;

        std::string get_physfs_error(const std::string& context, const std::filesystem::path& path) const;

        std::expected<std::filesystem::path, std::string> resolve_physical_write_path(const std::filesystem::path& virtual_path) const;

        std::unordered_map<std::string, std::filesystem::path> m_writable_mounts;
    };

} // namespace sandbox::modules
