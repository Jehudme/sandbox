#pragma once

#include <sandbox/abi/filesystem.h>
#include <flecs.h>
#include <string>
#include <vector>
#include <cstdint>
#include <unordered_map>

#include "miniz.h"

namespace sandbox::modules {


    /**
     * @brief A global module that abstracts file I/O over physical directories and archives.
     */
    class filesystem_t {
    public:
        /**
         * @brief Initializes the filesystem module.
         * @param entity_world The flecs world.
         */
        filesystem_t(flecs::world& entity_world);
        
        /**
         * @brief Destroys the filesystem module.
         */
        ~filesystem_t();


        // Prevent copying/moving to guarantee isolated state
        filesystem_t(const filesystem_t&) = delete;
        filesystem_t& operator=(const filesystem_t&) = delete;

        // ==========================================
        // ARCHIVE & MOUNTING
        // ==========================================
        bool mount(const char* physical_path, const char* virtual_mount_point, bool read_only = true); // should automaticly detect .zip and mount correctly
        bool unmount(const char* mount_point);

        // ==========================================
        // STREAMING FILE I/O (Handle-based)
        // ==========================================
        sandbox_file_handle_t open_read(const char* virtual_path);
        sandbox_file_handle_t open_write(const char* virtual_path, bool append = false, bool force_path = false);

        size_t read(sandbox_file_handle_t handle, void* buffer, size_t bytes_to_read);
        size_t write(sandbox_file_handle_t handle, const void* buffer, size_t bytes_to_write);

        bool eof(sandbox_file_handle_t handle) const;
        size_t tell(sandbox_file_handle_t handle) const;
        bool seek(sandbox_file_handle_t handle, size_t position);
        size_t size(sandbox_file_handle_t handle) const;
        void close(sandbox_file_handle_t handle);

        // ==========================================
        // ONE-SHOT I/O
        // ==========================================
        std::vector<uint8_t> read_all_bytes(const char* virtual_path);
        std::string read_all_text(const char* virtual_path);
        bool write_all(const char* virtual_path, const void* data, size_t size, bool force_path = false);

        // ==========================================
        // FILE MANIPULATION
        // ==========================================
        bool create_file(const char* virtual_path, bool force_path = false);
        bool remove_file(const char* virtual_path);
        bool copy(const char* source_virtual_path, const char* dest_virtual_path, bool overwrite = false, bool force_path = false);
        bool move(const char* source_virtual_path, const char* dest_virtual_path, bool overwrite = false, bool force_path = false);

        // ==========================================
        // DIRECTORY OPERATIONS
        // ==========================================
        bool create_directory(const char* virtual_path, bool force_path = false);
        bool remove_directory(const char* virtual_path);
        std::vector<std::string> list_contents(const char* virtual_path) const;

        // ==========================================
        // METADATA & UTILITIES
        // ==========================================
        bool exists(const char* virtual_path) const;
        bool is_file(const char* virtual_path) const;
        bool is_directory(const char* virtual_path) const;
        bool is_readonly(const char* virtual_path) const;
        size_t file_size(const char* virtual_path) const;
        int64_t last_modified(const char* virtual_path) const;

    private:
        flecs::world m_entity_world;

        std::unordered_map<std::string, std::string> m_physical_mounts;
    };

} // namespace sandbox::modules