#pragma once
#include "sandbox/abi/bootstrapper.h"
#ifdef __cplusplus
#include <vector>
#include <string>
#include <flecs/addons/cpp/flecs.hpp>
#include <sandbox/sdk/properties.hpp>
#endif
#include <sandbox/abi/bootstrapper.h>
#include <sandbox/abi/properties.h>
#include <sandbox/abi/handle.h>

extern "C" {
    /**
     * @brief An opaque handle representing an open file.
     */
    SANDBOX_DEFINE_HANDLE(sandbox_file_handle_t);

    /**
     * @brief API for the filesystem service.
     */
    typedef struct sandbox_filesystem_api_t {
        bool (*mount)(ecs_world_t* ecs, const char* physical_path, const char* virtual_mount_point, bool read_only);
        bool (*unmount)(ecs_world_t* ecs, const char* mount_point);
        sandbox_file_handle_t (*open_read)(ecs_world_t* ecs, const char* virtual_path);
        sandbox_file_handle_t (*open_write)(ecs_world_t* ecs, const char* virtual_path, bool append, bool force_path);
        size_t (*read)(ecs_world_t* ecs, sandbox_file_handle_t handle, void* buffer, size_t bytes_to_read);
        size_t (*write)(ecs_world_t* ecs, sandbox_file_handle_t handle, const void* buffer, size_t bytes_to_write);
        bool (*eof)(ecs_world_t* ecs, sandbox_file_handle_t handle);
        size_t (*tell)(ecs_world_t* ecs, sandbox_file_handle_t handle);
        bool (*seek)(ecs_world_t* ecs, sandbox_file_handle_t handle, size_t position);
        size_t (*size)(ecs_world_t* ecs, sandbox_file_handle_t handle);
        void (*close_handle)(ecs_world_t* ecs, sandbox_file_handle_t handle);
        bool (*create_file)(ecs_world_t* ecs, const char* virtual_path, bool force_path);
        bool (*remove_file)(ecs_world_t* ecs, const char* virtual_path);
        bool (*copy)(ecs_world_t* ecs, const char* source_virtual_path, const char* dest_virtual_path, bool overwrite, bool force_path);
        bool (*move)(ecs_world_t* ecs, const char* source_virtual_path, const char* dest_virtual_path, bool overwrite, bool force_path);
        bool (*create_directory)(ecs_world_t* ecs, const char* virtual_path, bool force_path);
        bool (*remove_directory)(ecs_world_t* ecs, const char* virtual_path);
        bool (*exists)(ecs_world_t* ecs, const char* virtual_path);
        bool (*is_file)(ecs_world_t* ecs, const char* virtual_path);
        bool (*is_directory)(ecs_world_t* ecs, const char* virtual_path);
        bool (*is_readonly)(ecs_world_t* ecs, const char* virtual_path);
        size_t (*file_size)(ecs_world_t* ecs, const char* virtual_path);
        int64_t (*last_modified)(ecs_world_t* ecs, const char* virtual_path);
        bool (*list_files)(ecs_world_t* ecs, const char* virtual_path, bool recursive, char*** out_files, size_t* out_count);
        void (*free_file_list)(ecs_world_t* ecs, char** files, size_t count);
        bool (*read_all_bytes)(ecs_world_t* ecs, const char* virtual_path, uint8_t** out_data, size_t* out_size);
        void (*free_bytes)(ecs_world_t* ecs, uint8_t* data);
    } sandbox_filesystem_api_t;

    /**
     * @brief The filesystem service definition.
     */
    SANDBOX_DECLARE_SERVICE(sandbox_filesystem_service_t, sandbox_filesystem_api_t, {
        .struct_size = 0,
        .name = "filesystem",
        .description = "Filesystem module service",
        .architecture = "sandbox::core",
        .version_major = 1,
        .version_minor = 0,
        .init_fn = NULL
    });
}

// --- Public C API ---
SANDBOX_API bool sandbox_filesystem_mount(ecs_world_t* ecs, const char* physical_path, const char* virtual_mount_point, bool read_only);
SANDBOX_API bool sandbox_filesystem_unmount(ecs_world_t* ecs, const char* mount_point);
SANDBOX_API sandbox_file_handle_t sandbox_filesystem_open_read(ecs_world_t* ecs, const char* virtual_path);
SANDBOX_API sandbox_file_handle_t sandbox_filesystem_open_write(ecs_world_t* ecs, const char* virtual_path, bool append, bool force_path);
SANDBOX_API size_t sandbox_filesystem_read(ecs_world_t* ecs, sandbox_file_handle_t handle, void* buffer, size_t bytes_to_read);
SANDBOX_API size_t sandbox_filesystem_write(ecs_world_t* ecs, sandbox_file_handle_t handle, const void* buffer, size_t bytes_to_write);
SANDBOX_API bool sandbox_filesystem_eof(ecs_world_t* ecs, sandbox_file_handle_t handle);
SANDBOX_API size_t sandbox_filesystem_tell(ecs_world_t* ecs, sandbox_file_handle_t handle);
SANDBOX_API bool sandbox_filesystem_seek(ecs_world_t* ecs, sandbox_file_handle_t handle, size_t position);
SANDBOX_API size_t sandbox_filesystem_size(ecs_world_t* ecs, sandbox_file_handle_t handle);
SANDBOX_API void sandbox_filesystem_close_handle(ecs_world_t* ecs, sandbox_file_handle_t handle);
SANDBOX_API bool sandbox_filesystem_create_file(ecs_world_t* ecs, const char* virtual_path, bool force_path);
SANDBOX_API bool sandbox_filesystem_remove_file(ecs_world_t* ecs, const char* virtual_path);
SANDBOX_API bool sandbox_filesystem_copy(ecs_world_t* ecs, const char* source_virtual_path, const char* dest_virtual_path, bool overwrite, bool force_path);
SANDBOX_API bool sandbox_filesystem_move(ecs_world_t* ecs, const char* source_virtual_path, const char* dest_virtual_path, bool overwrite, bool force_path);
SANDBOX_API bool sandbox_filesystem_create_directory(ecs_world_t* ecs, const char* virtual_path, bool force_path);
SANDBOX_API bool sandbox_filesystem_remove_directory(ecs_world_t* ecs, const char* virtual_path);
SANDBOX_API bool sandbox_filesystem_exists(ecs_world_t* ecs, const char* virtual_path);
SANDBOX_API bool sandbox_filesystem_is_file(ecs_world_t* ecs, const char* virtual_path);
SANDBOX_API bool sandbox_filesystem_is_directory(ecs_world_t* ecs, const char* virtual_path);
SANDBOX_API bool sandbox_filesystem_is_readonly(ecs_world_t* ecs, const char* virtual_path);
SANDBOX_API size_t sandbox_filesystem_file_size(ecs_world_t* ecs, const char* virtual_path);
SANDBOX_API int64_t sandbox_filesystem_last_modified(ecs_world_t* ecs, const char* virtual_path);
SANDBOX_API bool sandbox_filesystem_list_files(ecs_world_t* ecs, const char* virtual_path, bool recursive, char*** out_files, size_t* out_count);
SANDBOX_API void sandbox_filesystem_free_file_list(ecs_world_t* ecs, char** files, size_t count);
SANDBOX_API bool sandbox_filesystem_read_all_bytes(ecs_world_t* ecs, const char* virtual_path, uint8_t** out_data, size_t* out_size);
SANDBOX_API void sandbox_filesystem_free_bytes(ecs_world_t* ecs, uint8_t* data);
