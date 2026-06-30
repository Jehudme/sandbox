//
// Created by jehud on 2026-06-29.
//

#include "filesystem.h"
#include <sandbox/sdk/logs.hpp>
#include <sandbox/sdk/configuration.hpp>
#include "../../../sandbox/source/core/exceptions.h"
#include "miniz.h"

namespace sandbox::modules {

    filesystem::filesystem(flecs::world& ecs) : m_ecs(ecs) {
        sandbox::modules::logs::trace(m_ecs, "Filesystem Module Initializing...");

        // Require configuration service
        sandbox::properties config = sandbox::modules::configuration::get_properties(m_ecs);
        
        if (config.is_valid() && config.has("filesystem/mounts")) {
            std::vector<std::string> mounts = config.keys("filesystem/mounts");
            for (const auto& mnt_name : mounts) {
                std::string path = "filesystem/mounts/" + mnt_name;
                auto physical = config.get<std::string>(path + "/physical");
                auto readonly = config.get<bool>(path + "/readonly").value_or(true);
                
                if (physical.has_value()) {
                    // Virtual path is just the mount name preceded by /
                    std::string virtual_path = "/" + mnt_name;
                    mount(physical->c_str(), virtual_path.c_str(), readonly);
                }
            }
        }
    }

    filesystem::~filesystem() = default;

    bool filesystem::mount(const char* physical_path, const char* virtual_mount_point, bool read_only) {
        if (!physical_path || !virtual_mount_point) {
            sandbox::modules::logs::error(m_ecs, "Mount failed: null path provided");
            throw sandbox::core::filesystem_error("Null path provided to mount");
        }

        std::string path_str = physical_path;
        if (path_str.length() >= 4 && path_str.substr(path_str.length() - 4) == ".zip") {
            sandbox::modules::logs::info(m_ecs, "Mounting ZIP archive '{}' to '{}' (readonly: {})", physical_path, virtual_mount_point, read_only);
            
            // Validate the zip file using miniz
            mz_zip_archive zip_archive;
            memset(&zip_archive, 0, sizeof(zip_archive));
            
            if (!mz_zip_reader_init_file(&zip_archive, physical_path, 0)) {
                sandbox::modules::logs::error(m_ecs, "Failed to open zip file '{}'", physical_path);
                throw sandbox::core::filesystem_error("Failed to open zip file");
            }
            
            mz_zip_reader_end(&zip_archive);
        } else {
            sandbox::modules::logs::info(m_ecs, "Mounting physical include '{}' to '{}' (readonly: {})", physical_path, virtual_mount_point, read_only);
        }

        m_physical_mounts[virtual_mount_point] = physical_path;
        return true;
    }

    bool filesystem::unmount(const char* mount_point) { return false; }
    sandbox_file_handle_t filesystem::open_read(const char* virtual_path) { return {0}; }
    sandbox_file_handle_t filesystem::open_write(const char* virtual_path, bool append, bool force_path) { return {0}; }
    size_t filesystem::read(sandbox_file_handle_t handle, void* buffer, size_t bytes_to_read) { return 0; }
    size_t filesystem::write(sandbox_file_handle_t handle, const void* buffer, size_t bytes_to_write) { return 0; }
    bool filesystem::eof(sandbox_file_handle_t handle) const { return true; }
    size_t filesystem::tell(sandbox_file_handle_t handle) const { return 0; }
    bool filesystem::seek(sandbox_file_handle_t handle, size_t position) { return false; }
    size_t filesystem::size(sandbox_file_handle_t handle) const { return 0; }
    void filesystem::close(sandbox_file_handle_t handle) {}
    std::vector<uint8_t> filesystem::read_all_bytes(const char* virtual_path) { return {}; }
    std::string filesystem::read_all_text(const char* virtual_path) { return ""; }
    bool filesystem::write_all(const char* virtual_path, const void* data, size_t size, bool force_path) { return false; }
    bool filesystem::create_file(const char* virtual_path, bool force_path) { return false; }
    bool filesystem::remove_file(const char* virtual_path) { return false; }
    bool filesystem::copy(const char* source_virtual_path, const char* dest_virtual_path, bool overwrite, bool force_path) { return false; }
    bool filesystem::move(const char* source_virtual_path, const char* dest_virtual_path, bool overwrite, bool force_path) { return false; }
    bool filesystem::create_directory(const char* virtual_path, bool force_path) { return false; }
    bool filesystem::remove_directory(const char* virtual_path) { return false; }
    std::vector<std::string> filesystem::list_contents(const char* virtual_path) const { return {}; }
    bool filesystem::exists(const char* virtual_path) const { return false; }
    bool filesystem::is_file(const char* virtual_path) const { return false; }
    bool filesystem::is_directory(const char* virtual_path) const { return false; }
    bool filesystem::is_readonly(const char* virtual_path) const { return false; }
    size_t filesystem::file_size(const char* virtual_path) const { return 0; }
    int64_t filesystem::last_modified(const char* virtual_path) const { return 0; }
}
