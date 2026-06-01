#pragma once

#include "sandbox/core/ecs.h"
#include "sandbox/events/vfs.h"
#include <string>
#include <filesystem>

namespace sandbox::modules {

    class filesystem {
    public:
        filesystem(world& ecs);
        ~filesystem();

    private:
        void on_mount(world& ecs, const events::vfs::mount_path& e);
        void on_unmount(world& ecs, const events::vfs::unmount_path& e);
        void on_read(world& ecs, const events::vfs::read_request& e);
        void on_write(world& ecs, const events::vfs::write_request& e);
        void on_list(world& ecs, const events::vfs::list_request& e);
        void on_delete(world& ecs, const events::vfs::delete_request& e);
        void on_mkdir(world& ecs, const events::vfs::mkdir_request& e);
        void on_rename(world& ecs, const events::vfs::rename_request& e);
        void on_state(world& ecs, const events::vfs::state_request& e);
        void on_absolute(world& ecs, const events::vfs::absolute_request& e);

        std::string clean_path(const std::filesystem::path& path) const;
        void log_physfs_error(world& ecs, const std::string& context, const std::string& path) const;
    };

} // namespace sandbox::modules