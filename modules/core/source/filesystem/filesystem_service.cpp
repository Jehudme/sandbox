#include <sandbox/sdk/filesystem.hpp>
#include "sandbox/services/filesystem_service.h"
#include "filesystem_module.h"
#include <cstring>
#include <sandbox/sdk/logs.hpp>
#include <flecs.h>

// C-ABI Endpoints
// ==========================================

extern "C" {
    static bool filesystem_mount(ecs_world_t* entity_world, const char* physical_path, const char* virtual_mount_point, bool read_only) {
        flecs::world flecs_world(entity_world);
        auto* fs = flecs_world.try_get_mut<sandbox::modules::filesystem_t>();
        if (fs) {
            try {
                return fs->mount(physical_path, virtual_mount_point, read_only);
            } catch(...) {
                return false;
            }
        }
        return false;
    }
    static bool filesystem_unmount(ecs_world_t* entity_world, const char* mount_point) {
        flecs::world flecs_world(entity_world);
        auto* fs = flecs_world.try_get_mut<sandbox::modules::filesystem_t>();
        if (fs) {
            try {
                return fs->unmount(mount_point);
            } catch(...) {
                return false;
            }
        }
        return false;
    }
    static sandbox_file_handle_t filesystem_open_read(ecs_world_t* entity_world, const char* virtual_path) {
        flecs::world flecs_world(entity_world);
        auto* fs = flecs_world.try_get_mut<sandbox::modules::filesystem_t>();
        if (fs) {
            try {
                return fs->open_read(virtual_path);
            } catch(...) {
                return {0};
            }
        }
        return {0};
    }
    static sandbox_file_handle_t filesystem_open_write(ecs_world_t* entity_world, const char* virtual_path, bool append, bool force_path) {
        flecs::world flecs_world(entity_world);
        auto* fs = flecs_world.try_get_mut<sandbox::modules::filesystem_t>();
        if (fs) {
            try {
                return fs->open_write(virtual_path, append, force_path);
            } catch(...) {
                return {0};
            }
        }
        return {0};
    }
    static size_t filesystem_read(ecs_world_t* entity_world, sandbox_file_handle_t handle, void* buffer, size_t bytes_to_read) {
        flecs::world flecs_world(entity_world);
        auto* fs = flecs_world.try_get_mut<sandbox::modules::filesystem_t>();
        if (fs) {
            try {
                return fs->read(handle, buffer, bytes_to_read);
            } catch(...) {
                return 0;
            }
        }
        return 0;
    }
    static size_t filesystem_write(ecs_world_t* entity_world, sandbox_file_handle_t handle, const void* buffer, size_t bytes_to_write) {
        flecs::world flecs_world(entity_world);
        auto* fs = flecs_world.try_get_mut<sandbox::modules::filesystem_t>();
        if (fs) {
            try {
                return fs->write(handle, buffer, bytes_to_write);
            } catch(...) {
                return 0;
            }
        }
        return 0;
    }
    static bool filesystem_eof(ecs_world_t* entity_world, sandbox_file_handle_t handle) {
        flecs::world flecs_world(entity_world);
        auto* fs = flecs_world.try_get_mut<sandbox::modules::filesystem_t>();
        if (fs) {
            try {
                return fs->eof(handle);
            } catch(...) {
                return true;
            }
        }
        return true;
    }
    static size_t filesystem_tell(ecs_world_t* entity_world, sandbox_file_handle_t handle) {
        flecs::world flecs_world(entity_world);
        auto* fs = flecs_world.try_get_mut<sandbox::modules::filesystem_t>();
        if (fs) {
            try {
                return fs->tell(handle);
            } catch(...) {
                return 0;
            }
        }
        return 0;
    }
    static bool filesystem_seek(ecs_world_t* entity_world, sandbox_file_handle_t handle, size_t position) {
        flecs::world flecs_world(entity_world);
        auto* fs = flecs_world.try_get_mut<sandbox::modules::filesystem_t>();
        if (fs) {
            try {
                return fs->seek(handle, position);
            } catch(...) {
                return false;
            }
        }
        return false;
    }
    static size_t filesystem_size(ecs_world_t* entity_world, sandbox_file_handle_t handle) {
        flecs::world flecs_world(entity_world);
        auto* fs = flecs_world.try_get_mut<sandbox::modules::filesystem_t>();
        if (fs) {
            try {
                return fs->size(handle);
            } catch(...) {
                return 0;
            }
        }
        return 0;
    }
    static void filesystem_close_handle(ecs_world_t* entity_world, sandbox_file_handle_t handle) {
        flecs::world flecs_world(entity_world);
        auto* fs = flecs_world.try_get_mut<sandbox::modules::filesystem_t>();
        if (fs) {
            try {
                fs->close(handle);
            } catch(...) {
                
            }
        }
        
    }
    static bool filesystem_create_file(ecs_world_t* entity_world, const char* virtual_path, bool force_path) {
        flecs::world flecs_world(entity_world);
        auto* fs = flecs_world.try_get_mut<sandbox::modules::filesystem_t>();
        if (fs) {
            try {
                return fs->create_file(virtual_path, force_path);
            } catch(...) {
                return false;
            }
        }
        return false;
    }
    static bool filesystem_remove_file(ecs_world_t* entity_world, const char* virtual_path) {
        flecs::world flecs_world(entity_world);
        auto* fs = flecs_world.try_get_mut<sandbox::modules::filesystem_t>();
        if (fs) {
            try {
                return fs->remove_file(virtual_path);
            } catch(...) {
                return false;
            }
        }
        return false;
    }
    static bool filesystem_copy(ecs_world_t* entity_world, const char* source_virtual_path, const char* dest_virtual_path, bool overwrite, bool force_path) {
        flecs::world flecs_world(entity_world);
        auto* fs = flecs_world.try_get_mut<sandbox::modules::filesystem_t>();
        if (fs) {
            try {
                return fs->copy(source_virtual_path, dest_virtual_path, overwrite, force_path);
            } catch(...) {
                return false;
            }
        }
        return false;
    }
    static bool filesystem_move(ecs_world_t* entity_world, const char* source_virtual_path, const char* dest_virtual_path, bool overwrite, bool force_path) {
        flecs::world flecs_world(entity_world);
        auto* fs = flecs_world.try_get_mut<sandbox::modules::filesystem_t>();
        if (fs) {
            try {
                return fs->move(source_virtual_path, dest_virtual_path, overwrite, force_path);
            } catch(...) {
                return false;
            }
        }
        return false;
    }
    static bool filesystem_create_directory(ecs_world_t* entity_world, const char* virtual_path, bool force_path) {
        flecs::world flecs_world(entity_world);
        auto* fs = flecs_world.try_get_mut<sandbox::modules::filesystem_t>();
        if (fs) {
            try {
                return fs->create_directory(virtual_path, force_path);
            } catch(...) {
                return false;
            }
        }
        return false;
    }
    static bool filesystem_remove_directory(ecs_world_t* entity_world, const char* virtual_path) {
        flecs::world flecs_world(entity_world);
        auto* fs = flecs_world.try_get_mut<sandbox::modules::filesystem_t>();
        if (fs) {
            try {
                return fs->remove_directory(virtual_path);
            } catch(...) {
                return false;
            }
        }
        return false;
    }
    static bool filesystem_exists(ecs_world_t* entity_world, const char* virtual_path) {
        flecs::world flecs_world(entity_world);
        auto* fs = flecs_world.try_get_mut<sandbox::modules::filesystem_t>();
        if (fs) {
            try {
                return fs->exists(virtual_path);
            } catch(...) {
                return false;
            }
        }
        return false;
    }
    static bool filesystem_is_file(ecs_world_t* entity_world, const char* virtual_path) {
        flecs::world flecs_world(entity_world);
        auto* fs = flecs_world.try_get_mut<sandbox::modules::filesystem_t>();
        if (fs) {
            try {
                return fs->is_file(virtual_path);
            } catch(...) {
                return false;
            }
        }
        return false;
    }
    static bool filesystem_is_directory(ecs_world_t* entity_world, const char* virtual_path) {
        flecs::world flecs_world(entity_world);
        auto* fs = flecs_world.try_get_mut<sandbox::modules::filesystem_t>();
        if (fs) {
            try {
                return fs->is_directory(virtual_path);
            } catch(...) {
                return false;
            }
        }
        return false;
    }
    static bool filesystem_is_readonly(ecs_world_t* entity_world, const char* virtual_path) {
        flecs::world flecs_world(entity_world);
        auto* fs = flecs_world.try_get_mut<sandbox::modules::filesystem_t>();
        if (fs) {
            try {
                return fs->is_readonly(virtual_path);
            } catch(...) {
                return false;
            }
        }
        return false;
    }
    static size_t filesystem_file_size(ecs_world_t* entity_world, const char* virtual_path) {
        flecs::world flecs_world(entity_world);
        auto* fs = flecs_world.try_get_mut<sandbox::modules::filesystem_t>();
        if (fs) {
            try {
                return fs->file_size(virtual_path);
            } catch(...) {
                return 0;
            }
        }
        return 0;
    }
    static int64_t filesystem_last_modified(ecs_world_t* entity_world, const char* virtual_path) {
        flecs::world flecs_world(entity_world);
        auto* fs = flecs_world.try_get_mut<sandbox::modules::filesystem_t>();
        if (fs) {
            try {
                return fs->last_modified(virtual_path);
            } catch(...) {
                return 0;
            }
        }
        return 0;
    }

    static bool filesystem_list_files(ecs_world_t* entity_world, const char* virtual_path, bool recursive, char*** out_files, size_t* out_count) {
        if (!entity_world || !virtual_path || !out_files || !out_count) return false;
        flecs::world flecs_world(entity_world);
        auto* fs = flecs_world.try_get_mut<sandbox::modules::filesystem_t>();
        if (fs) {
            try {
                auto files = fs->list_files(virtual_path, recursive);
                if (files.empty()) {
                    *out_files = nullptr;
                    *out_count = 0;
                    return true;
                }
                *out_count = files.size();
                *out_files = new char*[files.size()];
                for (size_t i = 0; i < files.size(); ++i) {
                    (*out_files)[i] = strdup(files[i].c_str());
                }
                return true;
            } catch (const std::exception& e) {
                sandbox::modules::logs::error(flecs_world, "ABI filesystem_list_files error: {}", e.what());
                return false;
            } catch (...) {
                return false;
            }
        }
        return false;
    }

    static void filesystem_free_file_list(ecs_world_t*, char** files, size_t count) {
        if (!files) return;
        for (size_t i = 0; i < count; ++i) {
            if (files[i]) free(files[i]);
        }
        delete[] files;
    }

    static bool filesystem_read_all_bytes(ecs_world_t* entity_world, const char* virtual_path, uint8_t** out_data, size_t* out_size) {
        if (!entity_world || !virtual_path || !out_data || !out_size) return false;
        flecs::world flecs_world(entity_world);
        auto* fs = flecs_world.try_get_mut<sandbox::modules::filesystem_t>();
        if (fs) {
            try {
                auto data = fs->read_all_bytes(virtual_path);
                if (data.empty()) {
                    *out_data = nullptr;
                    *out_size = 0;
                    return true;
                }
                *out_size = data.size();
                *out_data = new uint8_t[data.size()];
                std::memcpy(*out_data, data.data(), data.size());
                return true;
            } catch (const std::exception& e) {
                sandbox::modules::logs::error(flecs_world, "ABI filesystem_read_all_bytes error: {}", e.what());
                return false;
            } catch (...) {
                return false;
            }
        }
        return false;
    }

    static void filesystem_free_bytes(ecs_world_t*, uint8_t* data) {
        if (data) delete[] data;
    }

    static bool filesystem_list_directories(ecs_world_t* entity_world, const char* virtual_path, bool recursive, char*** out_dirs, size_t* out_count) {
        if (!entity_world || !virtual_path || !out_dirs || !out_count) return false;
        flecs::world flecs_world(entity_world);
        auto* fs = flecs_world.try_get_mut<sandbox::modules::filesystem_t>();
        if (fs) {
            try {
                auto dirs = fs->list_directories(virtual_path, recursive);
                if (dirs.empty()) {
                    *out_dirs = nullptr;
                    *out_count = 0;
                    return true;
                }
                *out_count = dirs.size();
                *out_dirs = new char*[dirs.size()];
                for (size_t i = 0; i < dirs.size(); ++i) {
                    (*out_dirs)[i] = strdup(dirs[i].c_str());
                }
                return true;
            } catch (const std::exception& e) {
                sandbox::modules::logs::error(flecs_world, "ABI filesystem_list_directories error: {}", e.what());
                return false;
            } catch (...) {
                return false;
            }
        }
        return false;
    }

    static bool filesystem_resolve_physical_path(ecs_world_t* entity_world, const char* virtual_path, char** out_path) {
        if (!entity_world || !virtual_path || !out_path) return false;
        flecs::world flecs_world(entity_world);
        const auto* fs = flecs_world.try_get<sandbox::modules::filesystem_t>();
        if (fs) {
            std::string res = fs->resolve_full_physical_path(virtual_path);
            if (!res.empty()) {
                *out_path = strdup(res.c_str());
                return true;
            }
            *out_path = nullptr;
            return true;
        }
        return false;
    }

    static void filesystem_free_string(ecs_world_t*, char* str) {
        if (str) free(str);
    }

    static bool filesystem_write_all_bytes(ecs_world_t* entity_world, const char* virtual_path, const void* data, size_t size) {
        if (!entity_world || !virtual_path || (!data && size > 0)) return false;
        flecs::world flecs_world(entity_world);
        auto* fs = flecs_world.try_get_mut<sandbox::modules::filesystem_t>();
        if (fs) {
            try {
                return fs->write_all_bytes(virtual_path, data, size);
            } catch (const std::exception& e) {
                sandbox::modules::logs::error(flecs_world, "ABI filesystem_write_all_bytes error: {}", e.what());
                return false;
            } catch (...) {
                return false;
            }
        }
        return false;
    }

    static sandbox_filesystem_api_t filesystem_api = {
        .mount = filesystem_mount,
        .unmount = filesystem_unmount,
        .open_read = filesystem_open_read,
        .open_write = filesystem_open_write,
        .read = filesystem_read,
        .write = filesystem_write,
        .eof = filesystem_eof,
        .tell = filesystem_tell,
        .seek = filesystem_seek,
        .size = filesystem_size,
        .close_handle = filesystem_close_handle,
        .create_file = filesystem_create_file,
        .remove_file = filesystem_remove_file,
        .copy = filesystem_copy,
        .move = filesystem_move,
        .create_directory = filesystem_create_directory,
        .remove_directory = filesystem_remove_directory,
        .exists = filesystem_exists,
        .is_file = filesystem_is_file,
        .is_directory = filesystem_is_directory,
        .is_readonly = filesystem_is_readonly,
        .file_size = filesystem_file_size,
        .last_modified = filesystem_last_modified,
        .list_files = filesystem_list_files,
        .list_directories = filesystem_list_directories,
        .free_file_list = filesystem_free_file_list,
        .read_all_bytes = filesystem_read_all_bytes,
        .write_all_bytes = filesystem_write_all_bytes,
        .free_bytes = filesystem_free_bytes,
        .resolve_physical_path = filesystem_resolve_physical_path,
        .free_string = filesystem_free_string,
    };

    SANDBOX_DEFINE_SERVICE(sandbox_filesystem_service_t, sandbox_filesystem_api_t, &filesystem_api);
}

#ifdef __cplusplus
extern "C" {
#endif

// --- Public C API Implementations ---
bool sandbox_filesystem_mount(ecs_world_t* ecs, const char* physical_path, const char* virtual_mount_point, bool read_only) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->mount) {
        return service->api->mount(ecs, physical_path, virtual_mount_point, read_only);
        
    } else {
        sandbox::modules::logs::error(flecs_world, "[Filesystem Module] Service not initialized!");
    }
    return false;
}

bool sandbox_filesystem_unmount(ecs_world_t* ecs, const char* mount_point) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->unmount) {
        return service->api->unmount(ecs, mount_point);
        
    } else {
        sandbox::modules::logs::error(flecs_world, "[Filesystem Module] Service not initialized!");
    }
    return false;
}

sandbox_file_handle_t sandbox_filesystem_open_read(ecs_world_t* ecs, const char* virtual_path) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->open_read) {
        return service->api->open_read(ecs, virtual_path);
        
    } else {
        sandbox::modules::logs::error(flecs_world, "[Filesystem Module] Service not initialized!");
    }
    return (sandbox_file_handle_t){0};
}

sandbox_file_handle_t sandbox_filesystem_open_write(ecs_world_t* ecs, const char* virtual_path, bool append, bool force_path) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->open_write) {
        return service->api->open_write(ecs, virtual_path, append, force_path);
        
    } else {
        sandbox::modules::logs::error(flecs_world, "[Filesystem Module] Service not initialized!");
    }
    return (sandbox_file_handle_t){0};
}

size_t sandbox_filesystem_read(ecs_world_t* ecs, sandbox_file_handle_t handle, void* buffer, size_t bytes_to_read) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->read) {
        return service->api->read(ecs, handle, buffer, bytes_to_read);
        
    } else {
        sandbox::modules::logs::error(flecs_world, "[Filesystem Module] Service not initialized!");
    }
    return 0;
}

size_t sandbox_filesystem_write(ecs_world_t* ecs, sandbox_file_handle_t handle, const void* buffer, size_t bytes_to_write) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->write) {
        return service->api->write(ecs, handle, buffer, bytes_to_write);
        
    } else {
        sandbox::modules::logs::error(flecs_world, "[Filesystem Module] Service not initialized!");
    }
    return 0;
}

bool sandbox_filesystem_eof(ecs_world_t* ecs, sandbox_file_handle_t handle) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->eof) {
        return service->api->eof(ecs, handle);
        
    } else {
        sandbox::modules::logs::error(flecs_world, "[Filesystem Module] Service not initialized!");
    }
    return false;
}

size_t sandbox_filesystem_tell(ecs_world_t* ecs, sandbox_file_handle_t handle) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->tell) {
        return service->api->tell(ecs, handle);
        
    } else {
        sandbox::modules::logs::error(flecs_world, "[Filesystem Module] Service not initialized!");
    }
    return 0;
}

bool sandbox_filesystem_seek(ecs_world_t* ecs, sandbox_file_handle_t handle, size_t position) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->seek) {
        return service->api->seek(ecs, handle, position);
        
    } else {
        sandbox::modules::logs::error(flecs_world, "[Filesystem Module] Service not initialized!");
    }
    return false;
}

size_t sandbox_filesystem_size(ecs_world_t* ecs, sandbox_file_handle_t handle) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->size) {
        return service->api->size(ecs, handle);
        
    } else {
        sandbox::modules::logs::error(flecs_world, "[Filesystem Module] Service not initialized!");
    }
    return 0;
}

void sandbox_filesystem_close_handle(ecs_world_t* ecs, sandbox_file_handle_t handle) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->close_handle) {
        service->api->close_handle(ecs, handle);
        return;
    } else {
        sandbox::modules::logs::error(flecs_world, "[Filesystem Module] Service not initialized!");
    }
    
}

bool sandbox_filesystem_create_file(ecs_world_t* ecs, const char* virtual_path, bool force_path) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->create_file) {
        return service->api->create_file(ecs, virtual_path, force_path);
        
    } else {
        sandbox::modules::logs::error(flecs_world, "[Filesystem Module] Service not initialized!");
    }
    return false;
}

bool sandbox_filesystem_remove_file(ecs_world_t* ecs, const char* virtual_path) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->remove_file) {
        return service->api->remove_file(ecs, virtual_path);
        
    } else {
        sandbox::modules::logs::error(flecs_world, "[Filesystem Module] Service not initialized!");
    }
    return false;
}

bool sandbox_filesystem_copy(ecs_world_t* ecs, const char* source_virtual_path, const char* dest_virtual_path, bool overwrite, bool force_path) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->copy) {
        return service->api->copy(ecs, source_virtual_path, dest_virtual_path, overwrite, force_path);
        
    } else {
        sandbox::modules::logs::error(flecs_world, "[Filesystem Module] Service not initialized!");
    }
    return false;
}

bool sandbox_filesystem_move(ecs_world_t* ecs, const char* source_virtual_path, const char* dest_virtual_path, bool overwrite, bool force_path) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->move) {
        return service->api->move(ecs, source_virtual_path, dest_virtual_path, overwrite, force_path);
        
    } else {
        sandbox::modules::logs::error(flecs_world, "[Filesystem Module] Service not initialized!");
    }
    return false;
}

bool sandbox_filesystem_create_directory(ecs_world_t* ecs, const char* virtual_path, bool force_path) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->create_directory) {
        return service->api->create_directory(ecs, virtual_path, force_path);
        
    } else {
        sandbox::modules::logs::error(flecs_world, "[Filesystem Module] Service not initialized!");
    }
    return false;
}

bool sandbox_filesystem_remove_directory(ecs_world_t* ecs, const char* virtual_path) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->remove_directory) {
        return service->api->remove_directory(ecs, virtual_path);
        
    } else {
        sandbox::modules::logs::error(flecs_world, "[Filesystem Module] Service not initialized!");
    }
    return false;
}

bool sandbox_filesystem_exists(ecs_world_t* ecs, const char* virtual_path) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->exists) {
        return service->api->exists(ecs, virtual_path);
        
    } else {
        sandbox::modules::logs::error(flecs_world, "[Filesystem Module] Service not initialized!");
    }
    return false;
}

bool sandbox_filesystem_is_file(ecs_world_t* ecs, const char* virtual_path) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->is_file) {
        return service->api->is_file(ecs, virtual_path);
        
    } else {
        sandbox::modules::logs::error(flecs_world, "[Filesystem Module] Service not initialized!");
    }
    return false;
}

bool sandbox_filesystem_is_directory(ecs_world_t* ecs, const char* virtual_path) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->is_directory) {
        return service->api->is_directory(ecs, virtual_path);
        
    } else {
        sandbox::modules::logs::error(flecs_world, "[Filesystem Module] Service not initialized!");
    }
    return false;
}

bool sandbox_filesystem_is_readonly(ecs_world_t* ecs, const char* virtual_path) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->is_readonly) {
        return service->api->is_readonly(ecs, virtual_path);
        
    } else {
        sandbox::modules::logs::error(flecs_world, "[Filesystem Module] Service not initialized!");
    }
    return false;
}

size_t sandbox_filesystem_file_size(ecs_world_t* ecs, const char* virtual_path) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->file_size) {
        return service->api->file_size(ecs, virtual_path);
        
    } else {
        sandbox::modules::logs::error(flecs_world, "[Filesystem Module] Service not initialized!");
    }
    return 0;
}

int64_t sandbox_filesystem_last_modified(ecs_world_t* ecs, const char* virtual_path) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->last_modified) {
        return service->api->last_modified(ecs, virtual_path);
        
    } else {
        sandbox::modules::logs::error(flecs_world, "[Filesystem Module] Service not initialized!");
    }
    return 0;
}

bool sandbox_filesystem_list_files(ecs_world_t* ecs, const char* virtual_path, bool recursive, char*** out_files, size_t* out_count) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->list_files) {
        return service->api->list_files(ecs, virtual_path, recursive, out_files, out_count);
        
    } else {
        sandbox::modules::logs::error(flecs_world, "[Filesystem Module] Service not initialized!");
    }
    return false;
}

bool sandbox_filesystem_list_directories(ecs_world_t* ecs, const char* virtual_path, bool recursive, char*** out_dirs, size_t* out_count) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->list_directories) {
        return service->api->list_directories(ecs, virtual_path, recursive, out_dirs, out_count);
        
    } else {
        sandbox::modules::logs::error(flecs_world, "[Filesystem Module] Service not initialized!");
    }
    return false;
}

void sandbox_filesystem_free_file_list(ecs_world_t* ecs, char** files, size_t count) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->free_file_list) {
        service->api->free_file_list(ecs, files, count);
        return;
    } else {
        sandbox::modules::logs::error(flecs_world, "[Filesystem Module] Service not initialized!");
    }
    
}

bool sandbox_filesystem_read_all_bytes(ecs_world_t* ecs, const char* virtual_path, uint8_t** out_data, size_t* out_size) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->read_all_bytes) {
        return service->api->read_all_bytes(ecs, virtual_path, out_data, out_size);
        
    } else {
        sandbox::modules::logs::error(flecs_world, "[Filesystem Module] Service not initialized!");
    }
    return false;
}

void sandbox_filesystem_free_bytes(ecs_world_t* ecs, uint8_t* data) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->free_bytes) {
        service->api->free_bytes(ecs, data);
        return;
    } else {
        sandbox::modules::logs::error(flecs_world, "[Filesystem Module] Service not initialized!");
    }
    
}

bool sandbox_filesystem_write_all_bytes(ecs_world_t* ecs, const char* virtual_path, const void* data, size_t size) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->write_all_bytes) {
        return service->api->write_all_bytes(ecs, virtual_path, data, size);
        
    } else {
        sandbox::modules::logs::error(flecs_world, "[Filesystem Module] Service not initialized!");
    }
    return false;
}

bool sandbox_filesystem_resolve_physical_path(ecs_world_t* ecs, const char* virtual_path, char** out_path) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->resolve_physical_path) {
        return service->api->resolve_physical_path(ecs, virtual_path, out_path);
    } else {
        sandbox::modules::logs::error(flecs_world, "[Filesystem Module] Service not initialized!");
    }
    return false;
}

void sandbox_filesystem_free_string(ecs_world_t* ecs, char* str) {
#ifdef __cplusplus
    flecs::world flecs_world(ecs);
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#else
    const sandbox_filesystem_service_t* service = (const sandbox_filesystem_service_t*)ecs_singleton_get(ecs, sandbox_filesystem_service_t);
#endif
    if (service && service->api && service->api->free_string) {
        service->api->free_string(ecs, str);
        return;
    } else {
        sandbox::modules::logs::error(flecs_world, "[Filesystem Module] Service not initialized!");
    }
}

// --- SDK Implementations ---
namespace sandbox::modules {
bool filesystem::mount(flecs::world& entity_world, const char* physical_path, const char* virtual_mount_point, bool read_only) {
            const auto* service = SANDBOX_GET_SERVICE(entity_world, sandbox_filesystem_service_t);
            if (service && service->api) {
                return service->api->mount(entity_world.c_ptr(), physical_path, virtual_mount_point, read_only);
            } else {
                sandbox::modules::logs::error(entity_world, "[Filesystem Module] Service not initialized!");
            }
            return false;
        }

bool filesystem::unmount(flecs::world& entity_world, const char* mount_point) {
            const auto* service = SANDBOX_GET_SERVICE(entity_world, sandbox_filesystem_service_t);
            if (service && service->api) {
                return service->api->unmount(entity_world.c_ptr(), mount_point);
            } else {
                sandbox::modules::logs::error(entity_world, "[Filesystem Module] Service not initialized!");
            }
            return false;
        }

sandbox_file_handle_t filesystem::open_read(flecs::world& entity_world, const char* virtual_path) {
            const auto* service = SANDBOX_GET_SERVICE(entity_world, sandbox_filesystem_service_t);
            if (service && service->api) {
                return service->api->open_read(entity_world.c_ptr(), virtual_path);
            } else {
                sandbox::modules::logs::error(entity_world, "[Filesystem Module] Service not initialized!");
            }
            return {0};
        }

sandbox_file_handle_t filesystem::open_write(flecs::world& entity_world, const char* virtual_path, bool append, bool force_path) {
            const auto* service = SANDBOX_GET_SERVICE(entity_world, sandbox_filesystem_service_t);
            if (service && service->api) {
                return service->api->open_write(entity_world.c_ptr(), virtual_path, append, force_path);
            } else {
                sandbox::modules::logs::error(entity_world, "[Filesystem Module] Service not initialized!");
            }
            return {0};
        }

size_t filesystem::read(flecs::world& entity_world, sandbox_file_handle_t handle, void* buffer, size_t bytes_to_read) {
            const auto* service = SANDBOX_GET_SERVICE(entity_world, sandbox_filesystem_service_t);
            if (service && service->api) {
                return service->api->read(entity_world.c_ptr(), handle, buffer, bytes_to_read);
            } else {
                sandbox::modules::logs::error(entity_world, "[Filesystem Module] Service not initialized!");
            }
            return 0;
        }

size_t filesystem::write(flecs::world& entity_world, sandbox_file_handle_t handle, const void* buffer, size_t bytes_to_write) {
            const auto* service = SANDBOX_GET_SERVICE(entity_world, sandbox_filesystem_service_t);
            if (service && service->api) {
                return service->api->write(entity_world.c_ptr(), handle, buffer, bytes_to_write);
            } else {
                sandbox::modules::logs::error(entity_world, "[Filesystem Module] Service not initialized!");
            }
            return 0;
        }

bool filesystem::eof(flecs::world& entity_world, sandbox_file_handle_t handle) {
            const auto* service = SANDBOX_GET_SERVICE(entity_world, sandbox_filesystem_service_t);
            if (service && service->api) {
                return service->api->eof(entity_world.c_ptr(), handle);
            } else {
                sandbox::modules::logs::error(entity_world, "[Filesystem Module] Service not initialized!");
            }
            return false;
        }

size_t filesystem::tell(flecs::world& entity_world, sandbox_file_handle_t handle) {
            const auto* service = SANDBOX_GET_SERVICE(entity_world, sandbox_filesystem_service_t);
            if (service && service->api) {
                return service->api->tell(entity_world.c_ptr(), handle);
            } else {
                sandbox::modules::logs::error(entity_world, "[Filesystem Module] Service not initialized!");
            }
            return 0;
        }

bool filesystem::seek(flecs::world& entity_world, sandbox_file_handle_t handle, size_t position) {
            const auto* service = SANDBOX_GET_SERVICE(entity_world, sandbox_filesystem_service_t);
            if (service && service->api) {
                return service->api->seek(entity_world.c_ptr(), handle, position);
            } else {
                sandbox::modules::logs::error(entity_world, "[Filesystem Module] Service not initialized!");
            }
            return false;
        }

size_t filesystem::size(flecs::world& entity_world, sandbox_file_handle_t handle) {
            const auto* service = SANDBOX_GET_SERVICE(entity_world, sandbox_filesystem_service_t);
            if (service && service->api) {
                return service->api->size(entity_world.c_ptr(), handle);
            } else {
                sandbox::modules::logs::error(entity_world, "[Filesystem Module] Service not initialized!");
            }
            return 0;
        }

void filesystem::close_handle(flecs::world& entity_world, sandbox_file_handle_t handle) {
            const auto* service = SANDBOX_GET_SERVICE(entity_world, sandbox_filesystem_service_t);
            if (service && service->api) {
                service->api->close_handle(entity_world.c_ptr(), handle);
            } else {
                sandbox::modules::logs::error(entity_world, "[Filesystem Module] Service not initialized!");
            }
        }

bool filesystem::create_file(flecs::world& entity_world, const char* virtual_path, bool force_path) {
            const auto* service = SANDBOX_GET_SERVICE(entity_world, sandbox_filesystem_service_t);
            if (service && service->api) {
                return service->api->create_file(entity_world.c_ptr(), virtual_path, force_path);
            } else {
                sandbox::modules::logs::error(entity_world, "[Filesystem Module] Service not initialized!");
            }
            return false;
        }

bool filesystem::remove_file(flecs::world& entity_world, const char* virtual_path) {
            const auto* service = SANDBOX_GET_SERVICE(entity_world, sandbox_filesystem_service_t);
            if (service && service->api) {
                return service->api->remove_file(entity_world.c_ptr(), virtual_path);
            } else {
                sandbox::modules::logs::error(entity_world, "[Filesystem Module] Service not initialized!");
            }
            return false;
        }

bool filesystem::copy(flecs::world& entity_world, const char* source_virtual_path, const char* dest_virtual_path, bool overwrite, bool force_path) {
            const auto* service = SANDBOX_GET_SERVICE(entity_world, sandbox_filesystem_service_t);
            if (service && service->api) {
                return service->api->copy(entity_world.c_ptr(), source_virtual_path, dest_virtual_path, overwrite, force_path);
            } else {
                sandbox::modules::logs::error(entity_world, "[Filesystem Module] Service not initialized!");
            }
            return false;
        }

bool filesystem::move(flecs::world& entity_world, const char* source_virtual_path, const char* dest_virtual_path, bool overwrite, bool force_path) {
            const auto* service = SANDBOX_GET_SERVICE(entity_world, sandbox_filesystem_service_t);
            if (service && service->api) {
                return service->api->move(entity_world.c_ptr(), source_virtual_path, dest_virtual_path, overwrite, force_path);
            } else {
                sandbox::modules::logs::error(entity_world, "[Filesystem Module] Service not initialized!");
            }
            return false;
        }

bool filesystem::create_directory(flecs::world& entity_world, const char* virtual_path, bool force_path) {
            const auto* service = SANDBOX_GET_SERVICE(entity_world, sandbox_filesystem_service_t);
            if (service && service->api) {
                return service->api->create_directory(entity_world.c_ptr(), virtual_path, force_path);
            } else {
                sandbox::modules::logs::error(entity_world, "[Filesystem Module] Service not initialized!");
            }
            return false;
        }

bool filesystem::remove_directory(flecs::world& entity_world, const char* virtual_path) {
            const auto* service = SANDBOX_GET_SERVICE(entity_world, sandbox_filesystem_service_t);
            if (service && service->api) {
                return service->api->remove_directory(entity_world.c_ptr(), virtual_path);
            } else {
                sandbox::modules::logs::error(entity_world, "[Filesystem Module] Service not initialized!");
            }
            return false;
        }

bool filesystem::exists(flecs::world& entity_world, const char* virtual_path) {
            const auto* service = SANDBOX_GET_SERVICE(entity_world, sandbox_filesystem_service_t);
            if (service && service->api) {
                return service->api->exists(entity_world.c_ptr(), virtual_path);
            } else {
                sandbox::modules::logs::error(entity_world, "[Filesystem Module] Service not initialized!");
            }
            return false;
        }

bool filesystem::is_file(flecs::world& entity_world, const char* virtual_path) {
            const auto* service = SANDBOX_GET_SERVICE(entity_world, sandbox_filesystem_service_t);
            if (service && service->api) {
                return service->api->is_file(entity_world.c_ptr(), virtual_path);
            } else {
                sandbox::modules::logs::error(entity_world, "[Filesystem Module] Service not initialized!");
            }
            return false;
        }

bool filesystem::is_directory(flecs::world& entity_world, const char* virtual_path) {
            const auto* service = SANDBOX_GET_SERVICE(entity_world, sandbox_filesystem_service_t);
            if (service && service->api) {
                return service->api->is_directory(entity_world.c_ptr(), virtual_path);
            } else {
                sandbox::modules::logs::error(entity_world, "[Filesystem Module] Service not initialized!");
            }
            return false;
        }

bool filesystem::is_readonly(flecs::world& entity_world, const char* virtual_path) {
            const auto* service = SANDBOX_GET_SERVICE(entity_world, sandbox_filesystem_service_t);
            if (service && service->api) {
                return service->api->is_readonly(entity_world.c_ptr(), virtual_path);
            } else {
                sandbox::modules::logs::error(entity_world, "[Filesystem Module] Service not initialized!");
            }
            return false;
        }

size_t filesystem::file_size(flecs::world& entity_world, const char* virtual_path) {
            const auto* service = SANDBOX_GET_SERVICE(entity_world, sandbox_filesystem_service_t);
            if (service && service->api) {
                return service->api->file_size(entity_world.c_ptr(), virtual_path);
            } else {
                sandbox::modules::logs::error(entity_world, "[Filesystem Module] Service not initialized!");
            }
            return 0;
        }

int64_t filesystem::last_modified(flecs::world& entity_world, const char* virtual_path) {
            const auto* service = SANDBOX_GET_SERVICE(entity_world, sandbox_filesystem_service_t);
            if (service && service->api) {
                return service->api->last_modified(entity_world.c_ptr(), virtual_path);
            } else {
                sandbox::modules::logs::error(entity_world, "[Filesystem Module] Service not initialized!");
            }
            return 0;
        }

std::string filesystem::read_all_text(flecs::world& entity_world, const char* virtual_path) {
            std::vector<uint8_t> bytes = read_all_bytes(entity_world, virtual_path);
            return std::string(bytes.begin(), bytes.end());
        }

bool filesystem::write_all(flecs::world& entity_world, const char* virtual_path, const void* data, size_t sz, bool force_path) {
            const auto* service = SANDBOX_GET_SERVICE(entity_world, sandbox_filesystem_service_t);
            if (service && service->api && service->api->write_all_bytes) {
                return service->api->write_all_bytes(entity_world.c_ptr(), virtual_path, data, sz);
            }
            return false;
        }
}

#ifdef __cplusplus
}
#endif
