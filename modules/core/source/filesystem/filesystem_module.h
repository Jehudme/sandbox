#pragma once

#include <sandbox/services/filesystem_service.h>
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


/**
         * @brief Copy constructor (deleted).
         */
        filesystem_t(const filesystem_t&) = delete;
/**
         * @brief Copy assignment operator (deleted).
         */
        filesystem_t& operator=(const filesystem_t&) = delete;

/**
         * @brief Mounts a physical directory or archive.
         * @param physical_path The physical path on disk.
         * @param virtual_mount_point The virtual path within the filesystem.
         * @param read_only True if read-only.
         * @return True on success.
         */
        bool mount(const char* physical_path, const char* virtual_mount_point, bool read_only = true);
/**
         * @brief Unmounts a virtual mount point.
         * @param mount_point The virtual path to unmount.
         * @return True on success.
         */
        bool unmount(const char* mount_point);

/**
         * @brief Opens a file for reading.
         * @param virtual_path The virtual file path.
         * @return A valid file handle, or invalid handle on failure.
         */
        sandbox_file_handle_t open_read(const char* virtual_path);
/**
         * @brief Opens a file for writing.
         * @param virtual_path The virtual file path.
         * @param append True to append, false to overwrite.
         * @param force_path True to create missing directories.
         * @return A valid file handle.
         */
        sandbox_file_handle_t open_write(const char* virtual_path, bool append = false, bool force_path = false);

/**
         * @brief Reads from a file handle.
         * @param handle The file handle.
         * @param buffer The buffer to read into.
         * @param bytes_to_read Number of bytes to read.
         * @return Number of bytes actually read.
         */
        size_t read(sandbox_file_handle_t handle, void* buffer, size_t bytes_to_read);
/**
         * @brief Writes to a file handle.
         * @param handle The file handle.
         * @param buffer The buffer to write from.
         * @param bytes_to_write Number of bytes to write.
         * @return Number of bytes actually written.
         */
        size_t write(sandbox_file_handle_t handle, const void* buffer, size_t bytes_to_write);

/**
         * @brief Checks if end-of-file is reached.
         * @param handle The file handle.
         * @return True if EOF.
         */
        bool eof(sandbox_file_handle_t handle) const;
/**
         * @brief Gets current position in file.
         * @param handle The file handle.
         * @return Position in bytes.
         */
        size_t tell(sandbox_file_handle_t handle) const;
/**
         * @brief Seeks to a position in file.
         * @param handle The file handle.
         * @param position The new position.
         * @return True on success.
         */
        bool seek(sandbox_file_handle_t handle, size_t position);
/**
         * @brief Gets total size of open file.
         * @param handle The file handle.
         * @return File size in bytes.
         */
        size_t size(sandbox_file_handle_t handle) const;
/**
         * @brief Closes a file handle.
         * @param handle The file handle.
         */
        void close(sandbox_file_handle_t handle);

/**
         * @brief Reads entire file into byte vector.
         * @param virtual_path The virtual file path.
         * @return File contents.
         */
        std::vector<uint8_t> read_all_bytes(const char* virtual_path);
/**
         * @brief Reads entire file into string.
         * @param virtual_path The virtual file path.
         * @return File contents.
         */
        std::string read_all_text(const char* virtual_path);
/**
         * @brief Writes data to a file.
         * @param virtual_path The virtual file path.
         * @param data The data to write.
         * @param size Size of data.
         * @param force_path Create missing directories.
         * @return True on success.
         */
        bool write_all(const char* virtual_path, const void* data, size_t size, bool force_path = false);

/**
         * @brief Creates an empty file.
         * @param virtual_path The virtual file path.
         * @param force_path Create missing directories.
         * @return True on success.
         */
        bool create_file(const char* virtual_path, bool force_path = false);
/**
         * @brief Removes a file.
         * @param virtual_path The virtual file path.
         * @return True on success.
         */
        bool remove_file(const char* virtual_path);
/**
         * @brief Copies a file.
         * @param source_virtual_path Source path.
         * @param dest_virtual_path Destination path.
         * @param overwrite True to overwrite existing.
         * @param force_path Create missing directories.
         * @return True on success.
         */
        bool copy(const char* source_virtual_path, const char* dest_virtual_path, bool overwrite = false, bool force_path = false);
/**
         * @brief Moves a file.
         * @param source_virtual_path Source path.
         * @param dest_virtual_path Destination path.
         * @param overwrite True to overwrite existing.
         * @param force_path Create missing directories.
         * @return True on success.
         */
        bool move(const char* source_virtual_path, const char* dest_virtual_path, bool overwrite = false, bool force_path = false);

/**
         * @brief Creates a directory.
         * @param virtual_path The directory path.
         * @param force_path Create intermediate directories.
         * @return True on success.
         */
        bool create_directory(const char* virtual_path, bool force_path = false);
/**
         * @brief Removes a directory.
         * @param virtual_path The directory path.
         * @return True on success.
         */
        bool remove_directory(const char* virtual_path);
/**
         * @brief Lists contents of a directory.
         * @param virtual_path The directory path.
         * @return List of file and directory names.
         */
        std::vector<std::string> list_contents(const char* virtual_path) const;
/**
         * @brief Lists all files inside a directory, optionally recursive.
         * @param virtual_path The directory path.
         * @param recursive True to search subdirectories recursively.
         * @return List of file paths relative to the virtual path.
         */
        std::vector<std::string> list_files(const char* virtual_path, bool recursive) const;


/**
         * @brief Checks if a path exists.
         * @param virtual_path The path.
         * @return True if exists.
         */
        bool exists(const char* virtual_path) const;
/**
         * @brief Checks if a path is a file.
         * @param virtual_path The path.
         * @return True if file.
         */
        bool is_file(const char* virtual_path) const;
/**
         * @brief Checks if a path is a directory.
         * @param virtual_path The path.
         * @return True if directory.
         */
        bool is_directory(const char* virtual_path) const;
/**
         * @brief Checks if a path is read-only.
         * @param virtual_path The path.
         * @return True if read-only.
         */
        bool is_readonly(const char* virtual_path) const;
/**
         * @brief Gets the size of a file.
         * @param virtual_path The file path.
         * @return File size in bytes.
         */
        size_t file_size(const char* virtual_path) const;
/**
         * @brief Gets the last modified time of a path.
         * @param virtual_path The path.
         * @return Timestamp.
         */
        int64_t last_modified(const char* virtual_path) const;

    private:
        flecs::world m_entity_world;

        std::unordered_map<std::string, std::string> m_physical_mounts;

        std::string resolve_physical_path(const std::string& virtual_path, std::string& out_internal_path) const;
    };

} // namespace sandbox::plugins