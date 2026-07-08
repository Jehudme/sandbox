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
        .free_file_list = filesystem_free_file_list,
        .read_all_bytes = filesystem_read_all_bytes,
        .free_bytes = filesystem_free_bytes,
    };

    SANDBOX_DEFINE_SERVICE(sandbox_filesystem_service_t, sandbox_filesystem_api_t, &filesystem_api);
}
