//
// Created by jehud on 2026-06-29.
//

#include "filesystem.h"
#include <sandbox/sdk/logs.hpp>
#include <sandbox/sdk/configuration.hpp>
#include "../../../sandbox/source/core/exceptions.h"
#include "miniz.h"

namespace sandbox::modules {

    filesystem_t::filesystem_t(flecs::world& entity_world) : m_entity_world(entity_world) {
        sandbox::modules::logs::trace(m_entity_world, "Filesystem Module Initializing...");

        // Require configuration service
        sandbox::properties config = sandbox::modules::configuration::get_properties(m_entity_world);
        
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

    filesystem_t::~filesystem_t() = default;

    bool filesystem_t::mount(const char* physical_path, const char* virtual_mount_point, bool read_only) {
        if (!physical_path || !virtual_mount_point) {
            sandbox::modules::logs::error(m_entity_world, "Mount failed: null path provided");
            throw sandbox::core::filesystem_error("Null path provided to mount");
        }

        std::string path_str = physical_path;
        if (path_str.length() >= 4 && path_str.substr(path_str.length() - 4) == ".zip") {
            sandbox::modules::logs::info(m_entity_world, "Mounting ZIP archive '{}' to '{}' (readonly: {})", physical_path, virtual_mount_point, read_only);
            
            // Validate the zip file using miniz
            mz_zip_archive zip_archive;
            memset(&zip_archive, 0, sizeof(zip_archive));
            
            if (!mz_zip_reader_init_file(&zip_archive, physical_path, 0)) {
                sandbox::modules::logs::error(m_entity_world, "Failed to open zip file '{}'", physical_path);
                throw sandbox::core::filesystem_error("Failed to open zip file");
            }
            
            mz_zip_reader_end(&zip_archive);
        } else {
            sandbox::modules::logs::info(m_entity_world, "Mounting physical include '{}' to '{}' (readonly: {})", physical_path, virtual_mount_point, read_only);
        }

        m_physical_mounts[virtual_mount_point] = physical_path;
        return true;
    }

    bool filesystem_t::unmount(const char* mount_point) { return false; }
    sandbox_file_handle_t filesystem_t::open_read(const char* virtual_path) { return {0}; }
    sandbox_file_handle_t filesystem_t::open_write(const char* virtual_path, bool append, bool force_path) { return {0}; }
    size_t filesystem_t::read(sandbox_file_handle_t handle, void* buffer, size_t bytes_to_read) { return 0; }
    size_t filesystem_t::write(sandbox_file_handle_t handle, const void* buffer, size_t bytes_to_write) { return 0; }
    bool filesystem_t::eof(sandbox_file_handle_t handle) const { return true; }
    size_t filesystem_t::tell(sandbox_file_handle_t handle) const { return 0; }
    bool filesystem_t::seek(sandbox_file_handle_t handle, size_t position) { return false; }
    size_t filesystem_t::size(sandbox_file_handle_t handle) const { return 0; }
    void filesystem_t::close(sandbox_file_handle_t handle) {}
    std::vector<uint8_t> filesystem_t::read_all_bytes(const char* virtual_path) { return {}; }
    std::string filesystem_t::read_all_text(const char* virtual_path) { return ""; }
    bool filesystem_t::write_all(const char* virtual_path, const void* data, size_t size, bool force_path) { return false; }
    bool filesystem_t::create_file(const char* virtual_path, bool force_path) { return false; }
    bool filesystem_t::remove_file(const char* virtual_path) { return false; }
    bool filesystem_t::copy(const char* source_virtual_path, const char* dest_virtual_path, bool overwrite, bool force_path) { return false; }
    bool filesystem_t::move(const char* source_virtual_path, const char* dest_virtual_path, bool overwrite, bool force_path) { return false; }
    bool filesystem_t::create_directory(const char* virtual_path, bool force_path) { return false; }
    bool filesystem_t::remove_directory(const char* virtual_path) { return false; }
    std::vector<std::string> filesystem_t::list_contents(const char* virtual_path) const { return {}; }
    bool filesystem_t::exists(const char* virtual_path) const { return false; }
    bool filesystem_t::is_file(const char* virtual_path) const { return false; }
    bool filesystem_t::is_directory(const char* virtual_path) const { return false; }
    bool filesystem_t::is_readonly(const char* virtual_path) const { return false; }
    size_t filesystem_t::file_size(const char* virtual_path) const { return 0; }
    int64_t filesystem_t::last_modified(const char* virtual_path) const { return 0; }
}


// ==========================================
// C-ABI Endpoints
// ==========================================
#include <sandbox/abi/filesystem.h>

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
    };

    SANDBOX_DEFINE_SERVICE(sandbox_filesystem_service_t, sandbox_filesystem_api_t, &filesystem_api);
}

namespace sandbox::modules {
    SANDBOX_DECLARE_MODULE(filesystem_t, {
        .name = "filesystem",
        .description = "Filesystem module",
        .architecture = "sandbox",
        .version_major = 1,
        .version_minor = 0,
        .version_patch = 0,
        .service = &sandbox_filesystem_service_t_info,
        .requirements = nullptr,
        .requirement_count = 0
    })
}
