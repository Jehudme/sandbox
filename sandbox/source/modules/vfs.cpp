#include "modules/vfs.h"

#include "sandbox/utilities/events.h"
#include "sandbox/macros/logger.h"
#include <physfs.h>

namespace sandbox::modules {

    filesystem::filesystem(world& ecs) {
        ecs.module<filesystem>("::Modules::Filesystem");

        if (!PHYSFS_init(nullptr)) {
            SANDBOX_FATAL_THROW(ecs, "[Filesystem] Failed to initialize PhysFS context layer.");
        }

        PHYSFS_permitSymbolicLinks(1);

        sandbox::events::subscribe<events::vfs::mount_path>(ecs, [this, &ecs](const auto& e) { on_mount(ecs, e); });
        sandbox::events::subscribe<events::vfs::unmount_path>(ecs, [this, &ecs](const auto& e) { on_unmount(ecs, e); });
        sandbox::events::subscribe<events::vfs::read_request>(ecs, [this, &ecs](const auto& e) { on_read(ecs, e); });
        sandbox::events::subscribe<events::vfs::write_request>(ecs, [this, &ecs](const auto& e) { on_write(ecs, e); });
        sandbox::events::subscribe<events::vfs::list_request>(ecs, [this, &ecs](const auto& e) { on_list(ecs, e); });
        sandbox::events::subscribe<events::vfs::delete_request>(ecs, [this, &ecs](const auto& e) { on_delete(ecs, e); });
        sandbox::events::subscribe<events::vfs::mkdir_request>(ecs, [this, &ecs](const auto& e) { on_mkdir(ecs, e); });
        sandbox::events::subscribe<events::vfs::rename_request>(ecs, [this, &ecs](const auto& e) { on_rename(ecs, e); });
        sandbox::events::subscribe<events::vfs::state_request>(ecs, [this, &ecs](const auto& e) { on_state(ecs, e); });
        sandbox::events::subscribe<events::vfs::absolute_request>(ecs, [this, &ecs](const auto& e) { on_absolute(ecs, e); });

        SANDBOX_INFO(ecs, "[Filesystem] Functional command factory subsystem operational.");
    }

    filesystem::~filesystem() {
        PHYSFS_deinit();
    }

    void filesystem::on_mount(world& ecs, const events::vfs::mount_path& e) {
        std::string phys = e.physical_path.string();
        std::string virt = clean_path(e.virtual_prefix);
        const char* mount_target = virt.empty() ? nullptr : virt.c_str();

        if (!PHYSFS_mount(phys.c_str(), mount_target, e.read_only ? 1 : 0)) {
            log_physfs_error(ecs, "Mount operation", phys);
        }
    }

    void filesystem::on_unmount(world& ecs, const events::vfs::unmount_path& e) {
        std::string virt = clean_path(e.virtual_prefix);
        if (!PHYSFS_unmount(virt.c_str())) {
            log_physfs_error(ecs, "Unmount operation", virt);
        }
    }

    void filesystem::on_read(world& ecs, const events::vfs::read_request& e) {
        std::string path = clean_path(e.virtual_path);

        e.result_command = [this, &ecs, path]() -> std::vector<std::byte> {
            std::vector<std::byte> buffer;
            PHYSFS_file* file = PHYSFS_openRead(path.c_str());
            if (!file) {
                log_physfs_error(ecs, "Open for Read", path);
                return buffer;
            }

            PHYSFS_sint64 len = PHYSFS_fileLength(file);
            buffer.resize(static_cast<size_t>(len));
            if (PHYSFS_readBytes(file, buffer.data(), len) < 0) {
                log_physfs_error(ecs, "Read Bytes", path);
                buffer.clear();
            }
            PHYSFS_close(file);
            return buffer;
        };
    }

    void filesystem::on_write(world& ecs, const events::vfs::write_request& e) {
        std::string path = clean_path(e.virtual_path);
        bool append = e.append_mode;

        e.result_command = [this, &ecs, path, append, data = std::move(e.data)]() -> bool {
            PHYSFS_file* file = append ? PHYSFS_openAppend(path.c_str()) : PHYSFS_openWrite(path.c_str());
            if (!file) {
                log_physfs_error(ecs, "Open for Write/Append", path);
                return false;
            }

            PHYSFS_sint64 written = PHYSFS_writeBytes(file, data.data(), data.size());
            bool success = (written == static_cast<PHYSFS_sint64>(data.size()));
            if (!success) {
                log_physfs_error(ecs, "Write Bytes", path);
            }
            PHYSFS_close(file);
            return success;
        };
    }

    void filesystem::on_list(world& ecs, const events::vfs::list_request& e) {
        std::string base_phys_path = clean_path(e.virtual_path);
        std::filesystem::path base_virt_path = e.virtual_path;
        bool recursive = e.recursive;

        e.result_command = [this, &ecs, base_phys_path, base_virt_path, recursive]() -> std::vector<std::filesystem::path> {
            std::vector<std::filesystem::path> total_paths;

            auto walk_directory = [&](auto& self, const std::string& current_phys, const std::filesystem::path& current_virt) -> void {
                char** files = PHYSFS_enumerateFiles(current_phys.c_str());
                if (!files) return;

                for (char** i = files; *i != nullptr; i++) {
                    std::string item_name = *i;
                    std::string next_phys = current_phys.empty() ? item_name : current_phys + "/" + item_name;
                    std::filesystem::path next_virt = current_virt / item_name;

                    total_paths.push_back(next_virt);

                    if (recursive) {
                        PHYSFS_Stat stat;
                        if (PHYSFS_stat(next_phys.c_str(), &stat) != 0 && stat.filetype == PHYSFS_FILETYPE_DIRECTORY) {
                            self(self, next_phys, next_virt);
                        }
                    }
                }
                PHYSFS_freeList(files);
            };

            walk_directory(walk_directory, base_phys_path, base_virt_path);
            return total_paths;
        };
    }

    void filesystem::on_delete(world& ecs, const events::vfs::delete_request& e) {
        std::string path = clean_path(e.virtual_path);
        e.result_command = [this, &ecs, path]() -> bool {
            bool success = (PHYSFS_delete(path.c_str()) != 0);
            if (!success) log_physfs_error(ecs, "Delete File/Folder", path);
            return success;
        };
    }

    void filesystem::on_mkdir(world& ecs, const events::vfs::mkdir_request& e) {
        std::string path = clean_path(e.virtual_path);
        e.result_command = [this, &ecs, path]() -> bool {
            bool success = (PHYSFS_mkdir(path.c_str()) != 0);
            if (!success) log_physfs_error(ecs, "Create Directory", path);
            return success;
        };
    }

    void filesystem::on_rename(world& ecs, const events::vfs::rename_request& e) {
        std::string old_p = clean_path(e.old_virtual_path);
        std::string new_p = clean_path(e.new_virtual_path);

        e.result_command = [&ecs, old_p, new_p]() -> bool {
            if (const char* real_dir = PHYSFS_getRealDir(old_p.c_str())) {
                std::filesystem::path physical_old = std::filesystem::path(real_dir) / old_p;
                std::filesystem::path physical_new = std::filesystem::path(real_dir) / new_p;
                std::error_code ec;
                std::filesystem::rename(physical_old, physical_new, ec);
                if (!ec) return true;
                SANDBOX_ERROR(ecs, "[Filesystem] OS Rename failed from '{}' to '{}' | Error: {}", old_p, new_p, ec.message());
            } else {
                SANDBOX_ERROR(ecs, "[Filesystem] Rename targeted a missing resource: '{}'", old_p);
            }
            return false;
        };
    }

    void filesystem::on_state(world& ecs, const events::vfs::state_request& e) {
        std::string path = clean_path(e.virtual_path);
        std::filesystem::path original_virt = e.virtual_path;

        e.result_command = [this, &ecs, path, original_virt]() -> events::vfs::file_metadata {
            events::vfs::file_metadata metadata{};
            metadata.virtual_path = original_virt;

            PHYSFS_Stat stat;
            // FIXED: == 0 means PhysFS failed to read
            if (PHYSFS_stat(path.c_str(), &stat) == 0) {
                log_physfs_error(ecs, "Query Stat Details", path);
                return metadata;
            }

            metadata.size = stat.filesize;
            metadata.creation_time = stat.createtime;
            metadata.modification_time = stat.modtime;
            metadata.access_time = stat.accesstime;
            metadata.read_only = (stat.readonly != 0);

            switch (stat.filetype) {
                case PHYSFS_FILETYPE_REGULAR:   metadata.type = events::vfs::file_type::regular;   break;
                case PHYSFS_FILETYPE_DIRECTORY: metadata.type = events::vfs::file_type::directory; break;
                case PHYSFS_FILETYPE_SYMLINK:   metadata.type = events::vfs::file_type::symlink;   break;
                default:                        metadata.type = events::vfs::file_type::unknown;   break;
            }

            return metadata;
        };
    }

    // NEW HANDLER: Instantly translates a Virtual Path to an Absolute OS Path
    void filesystem::on_absolute(world& ecs, const events::vfs::absolute_request& e) {
        std::string path = clean_path(e.virtual_path);

        e.result_command = [&ecs, path]() -> std::filesystem::path {
            if (const char* real_dir = PHYSFS_getRealDir(path.c_str())) {
                return std::filesystem::path(real_dir) / path;
            }
            return {}; // Returns an empty path if it doesn't physically exist
        };
    }

    // MASSIVELY SIMPLIFIED STRING STRIPPER (No messy loops)
    std::string filesystem::clean_path(const std::filesystem::path& path) const {
        std::string p = path.generic_string();

        size_t colon = p.find(':');
        if (colon != std::string::npos) {
            // Find the first character of the mount name (skipping ://)
            size_t mount_start = p.find_first_not_of('/', colon + 1);
            if (mount_start != std::string::npos) {
                // Find the slash immediately after the mount name
                size_t path_start = p.find('/', mount_start);
                if (path_start != std::string::npos) {
                    // Find the start of the actual relative subpath
                    size_t real_start = p.find_first_not_of('/', path_start);
                    if (real_start != std::string::npos) {
                        p = p.substr(real_start);
                    } else p = "";
                } else p = "";
            } else p = "";
        }

        // Strip trailing slashes
        while (!p.empty() && p.back() == '/') p.pop_back();

        return p;
    }

    void filesystem::log_physfs_error(world& ecs, const std::string& context, const std::string& path) const {
        PHYSFS_ErrorCode err_code = PHYSFS_getLastErrorCode();
        const char* err_desc = PHYSFS_getErrorByCode(err_code);
        SANDBOX_ERROR(ecs, "[Filesystem] {} failed for '{}' | Error: {}", context, path, err_desc);
    }

} // namespace sandbox::modules