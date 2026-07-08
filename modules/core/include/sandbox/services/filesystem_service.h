#pragma once
#include "sandbox/abi/bootstrapper.h"
#include <vector>
#include <string>
#include <flecs/addons/cpp/flecs.hpp>
#include <sandbox/sdk/properties.hpp>
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
static inline bool sandbox_filesystem_mount(ecs_world_t* ecs, const char* physical_path, const char* virtual_mount_point, bool read_only) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = flecs_world.try_get<sandbox_filesystem_service_t>();
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->mount) {
        return service->api->mount(ecs, physical_path, virtual_mount_point, read_only);
        
    }
    return false;
}
static inline bool sandbox_filesystem_unmount(ecs_world_t* ecs, const char* mount_point) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = flecs_world.try_get<sandbox_filesystem_service_t>();
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->unmount) {
        return service->api->unmount(ecs, mount_point);
        
    }
    return false;
}
static inline sandbox_file_handle_t sandbox_filesystem_open_read(ecs_world_t* ecs, const char* virtual_path) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = flecs_world.try_get<sandbox_filesystem_service_t>();
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->open_read) {
        return service->api->open_read(ecs, virtual_path);
        
    }
    return (sandbox_file_handle_t){0};
}
static inline sandbox_file_handle_t sandbox_filesystem_open_write(ecs_world_t* ecs, const char* virtual_path, bool append, bool force_path) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = flecs_world.try_get<sandbox_filesystem_service_t>();
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->open_write) {
        return service->api->open_write(ecs, virtual_path, append, force_path);
        
    }
    return (sandbox_file_handle_t){0};
}
static inline size_t sandbox_filesystem_read(ecs_world_t* ecs, sandbox_file_handle_t handle, void* buffer, size_t bytes_to_read) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = flecs_world.try_get<sandbox_filesystem_service_t>();
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->read) {
        return service->api->read(ecs, handle, buffer, bytes_to_read);
        
    }
    return 0;
}
static inline size_t sandbox_filesystem_write(ecs_world_t* ecs, sandbox_file_handle_t handle, const void* buffer, size_t bytes_to_write) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = flecs_world.try_get<sandbox_filesystem_service_t>();
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->write) {
        return service->api->write(ecs, handle, buffer, bytes_to_write);
        
    }
    return 0;
}
static inline bool sandbox_filesystem_eof(ecs_world_t* ecs, sandbox_file_handle_t handle) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = flecs_world.try_get<sandbox_filesystem_service_t>();
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->eof) {
        return service->api->eof(ecs, handle);
        
    }
    return false;
}
static inline size_t sandbox_filesystem_tell(ecs_world_t* ecs, sandbox_file_handle_t handle) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = flecs_world.try_get<sandbox_filesystem_service_t>();
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->tell) {
        return service->api->tell(ecs, handle);
        
    }
    return 0;
}
static inline bool sandbox_filesystem_seek(ecs_world_t* ecs, sandbox_file_handle_t handle, size_t position) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = flecs_world.try_get<sandbox_filesystem_service_t>();
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->seek) {
        return service->api->seek(ecs, handle, position);
        
    }
    return false;
}
static inline size_t sandbox_filesystem_size(ecs_world_t* ecs, sandbox_file_handle_t handle) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = flecs_world.try_get<sandbox_filesystem_service_t>();
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->size) {
        return service->api->size(ecs, handle);
        
    }
    return 0;
}
static inline void sandbox_filesystem_close_handle(ecs_world_t* ecs, sandbox_file_handle_t handle) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = flecs_world.try_get<sandbox_filesystem_service_t>();
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->close_handle) {
        service->api->close_handle(ecs, handle);
        return;
    }
    
}
static inline bool sandbox_filesystem_create_file(ecs_world_t* ecs, const char* virtual_path, bool force_path) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = flecs_world.try_get<sandbox_filesystem_service_t>();
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->create_file) {
        return service->api->create_file(ecs, virtual_path, force_path);
        
    }
    return false;
}
static inline bool sandbox_filesystem_remove_file(ecs_world_t* ecs, const char* virtual_path) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = flecs_world.try_get<sandbox_filesystem_service_t>();
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->remove_file) {
        return service->api->remove_file(ecs, virtual_path);
        
    }
    return false;
}
static inline bool sandbox_filesystem_copy(ecs_world_t* ecs, const char* source_virtual_path, const char* dest_virtual_path, bool overwrite, bool force_path) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = flecs_world.try_get<sandbox_filesystem_service_t>();
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->copy) {
        return service->api->copy(ecs, source_virtual_path, dest_virtual_path, overwrite, force_path);
        
    }
    return false;
}
static inline bool sandbox_filesystem_move(ecs_world_t* ecs, const char* source_virtual_path, const char* dest_virtual_path, bool overwrite, bool force_path) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = flecs_world.try_get<sandbox_filesystem_service_t>();
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->move) {
        return service->api->move(ecs, source_virtual_path, dest_virtual_path, overwrite, force_path);
        
    }
    return false;
}
static inline bool sandbox_filesystem_create_directory(ecs_world_t* ecs, const char* virtual_path, bool force_path) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = flecs_world.try_get<sandbox_filesystem_service_t>();
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->create_directory) {
        return service->api->create_directory(ecs, virtual_path, force_path);
        
    }
    return false;
}
static inline bool sandbox_filesystem_remove_directory(ecs_world_t* ecs, const char* virtual_path) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = flecs_world.try_get<sandbox_filesystem_service_t>();
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->remove_directory) {
        return service->api->remove_directory(ecs, virtual_path);
        
    }
    return false;
}
static inline bool sandbox_filesystem_exists(ecs_world_t* ecs, const char* virtual_path) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = flecs_world.try_get<sandbox_filesystem_service_t>();
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->exists) {
        return service->api->exists(ecs, virtual_path);
        
    }
    return false;
}
static inline bool sandbox_filesystem_is_file(ecs_world_t* ecs, const char* virtual_path) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = flecs_world.try_get<sandbox_filesystem_service_t>();
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->is_file) {
        return service->api->is_file(ecs, virtual_path);
        
    }
    return false;
}
static inline bool sandbox_filesystem_is_directory(ecs_world_t* ecs, const char* virtual_path) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = flecs_world.try_get<sandbox_filesystem_service_t>();
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->is_directory) {
        return service->api->is_directory(ecs, virtual_path);
        
    }
    return false;
}
static inline bool sandbox_filesystem_is_readonly(ecs_world_t* ecs, const char* virtual_path) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = flecs_world.try_get<sandbox_filesystem_service_t>();
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->is_readonly) {
        return service->api->is_readonly(ecs, virtual_path);
        
    }
    return false;
}
static inline size_t sandbox_filesystem_file_size(ecs_world_t* ecs, const char* virtual_path) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = flecs_world.try_get<sandbox_filesystem_service_t>();
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->file_size) {
        return service->api->file_size(ecs, virtual_path);
        
    }
    return 0;
}
static inline int64_t sandbox_filesystem_last_modified(ecs_world_t* ecs, const char* virtual_path) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = flecs_world.try_get<sandbox_filesystem_service_t>();
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->last_modified) {
        return service->api->last_modified(ecs, virtual_path);
        
    }
    return 0;
}
static inline bool sandbox_filesystem_list_files(ecs_world_t* ecs, const char* virtual_path, bool recursive, char*** out_files, size_t* out_count) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = flecs_world.try_get<sandbox_filesystem_service_t>();
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->list_files) {
        return service->api->list_files(ecs, virtual_path, recursive, out_files, out_count);
        
    }
    return false;
}
static inline void sandbox_filesystem_free_file_list(ecs_world_t* ecs, char** files, size_t count) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = flecs_world.try_get<sandbox_filesystem_service_t>();
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->free_file_list) {
        service->api->free_file_list(ecs, files, count);
        return;
    }
    
}
static inline bool sandbox_filesystem_read_all_bytes(ecs_world_t* ecs, const char* virtual_path, uint8_t** out_data, size_t* out_size) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = flecs_world.try_get<sandbox_filesystem_service_t>();
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->read_all_bytes) {
        return service->api->read_all_bytes(ecs, virtual_path, out_data, out_size);
        
    }
    return false;
}
static inline void sandbox_filesystem_free_bytes(ecs_world_t* ecs, uint8_t* data) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = flecs_world.try_get<sandbox_filesystem_service_t>();
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->free_bytes) {
        service->api->free_bytes(ecs, data);
        return;
    }
    
}
