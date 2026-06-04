#pragma once

#include "sandbox/core/ecs.h"
#include "sandbox/event_bus/filesystem_events.h"
#include <string>
#include <string_view>
#include <filesystem>
#include <unordered_map>

namespace sandbox::modules {

    class filesystem_module {
    public:
        filesystem_module(world& ecs);
        ~filesystem_module();

    private:
        void on_mount(world& ecs, const events::filesystem::mount_path& e);
        void on_unmount(world& ecs, const events::filesystem::unmount_path& e);
        void on_read(world& ecs, const events::filesystem::read_request& e);
        void on_write(world& ecs, const events::filesystem::write_request& e);
        void on_list(world& ecs, const events::filesystem::list_request& e);
        void on_delete(world& ecs, const events::filesystem::delete_request& e);
        void on_mkdir(world& ecs, const events::filesystem::mkdir_request& e);
        void on_rename(world& ecs, const events::filesystem::rename_request& e);
        void on_copy(world& ecs, const events::filesystem::copy_request& e);
        void on_move(world& ecs, const events::filesystem::move_request& e);
        void on_state(world& ecs, const events::filesystem::state_request& e);
        void on_absolute(world& ecs, const events::filesystem::absolute_request& e);

        std::string get_mount_prefix(std::string_view v_path) const;
        std::string get_sub_path(std::string_view v_path) const;
        std::string get_physfs_path(std::string_view v_path) const;

        void throw_physfs_error(const std::string& context, const std::filesystem::path& path) const;

        std::filesystem::path resolve_physical_write_path(const std::filesystem::path& virtual_path) const;

        std::unordered_map<std::string, std::filesystem::path> m_writable_mounts;
    };

} // namespace sandbox::modules
