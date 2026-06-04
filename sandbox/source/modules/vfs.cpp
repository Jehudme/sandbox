#include "modules/vfs.h"
#include "sandbox/exceptions/vfs_exceptions.h"
#include "sandbox/utilities/events.h"
#include "../../include/sandbox/core/logger.h"
#include <physfs.h>
#include <fstream>

namespace sandbox::modules {

    // MARK: - Subsystem Lifecycle

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
        sandbox::events::subscribe<events::vfs::copy_request>(ecs, [this, &ecs](const auto& e) { on_copy(ecs, e); });
        sandbox::events::subscribe<events::vfs::move_request>(ecs, [this, &ecs](const auto& e) { on_move(ecs, e); });
        sandbox::events::subscribe<events::vfs::state_request>(ecs, [this, &ecs](const auto& e) { on_state(ecs, e); });
        sandbox::events::subscribe<events::vfs::absolute_request>(ecs, [this, &ecs](const auto& e) { on_absolute(ecs, e); });

        SANDBOX_INFO(ecs, "[Filesystem] Functional command factory subsystem operational.");
    }

    filesystem::~filesystem() {
        PHYSFS_deinit();
    }

    // MARK: - Subsystem Implementation

    void filesystem::on_mount(world& ecs, const events::vfs::mount_path& e) {
        std::string v_str = e.virtual_prefix.generic_string();

        if (v_str.find(":/") == std::string::npos) {
            throw events::vfs::vfs_mount_error("Format Validation", e.virtual_prefix, "Missing protocol separator (e.g., 'mount://').");
        }

        std::string prefix = get_mount_prefix(v_str);
        if (prefix.empty()) {
            throw events::vfs::vfs_mount_error("Format Validation", e.virtual_prefix, "Mount name cannot be empty.");
        }

        if (!get_sub_path(v_str).empty()) {
            throw events::vfs::vfs_mount_error("Format Validation", e.virtual_prefix, "Mount target cannot contain sub-directories.");
        }

        std::error_code ec;
        std::filesystem::create_directories(e.physical_path, ec);

        std::string phys = e.physical_path.string();

        // ISOLATED TREE FIX: We mount it explicitly to its own prefix folder in PhysFS
        if (!PHYSFS_mount(phys.c_str(), prefix.c_str(), e.read_only ? 1 : 0)) {
            throw_physfs_error("Mount Operation", e.physical_path);
        }

        if (!e.read_only) {
            m_writable_mounts[prefix] = e.physical_path;
        }
    }

    void filesystem::on_unmount(world& ecs, const events::vfs::unmount_path& e) {
        std::string v_str = e.virtual_prefix.generic_string();
        std::string prefix = get_mount_prefix(v_str);

        // In modern PhysFS, unmount expects the physical path
        auto it = m_writable_mounts.find(prefix);
        if (it != m_writable_mounts.end()) {
            PHYSFS_unmount(it->second.string().c_str());
            m_writable_mounts.erase(it);
        }
    }

    void filesystem::on_read(world& ecs, const events::vfs::read_request& e) {
        std::string path = get_physfs_path(e.virtual_path.generic_string());

        e.result_command = [this, path, virt_path = e.virtual_path]() -> std::vector<std::byte> {
            PHYSFS_file* file = PHYSFS_openRead(path.c_str());
            if (!file) throw_physfs_error("Open for Read", virt_path);

            PHYSFS_sint64 len = PHYSFS_fileLength(file);
            std::vector<std::byte> buffer(static_cast<size_t>(len));

            if (PHYSFS_readBytes(file, buffer.data(), len) < 0) {
                PHYSFS_close(file);
                throw_physfs_error("Read Bytes", virt_path);
            }

            PHYSFS_close(file);
            return buffer;
        };
    }

    void filesystem::on_write(world& ecs, const events::vfs::write_request& e) {
        bool append = e.append_mode;

        e.result_command = [this, virt_path = e.virtual_path, append, data = std::move(e.data)]() -> void {
            std::filesystem::path physical_target = resolve_physical_write_path(virt_path);

            std::error_code ec;
            std::filesystem::create_directories(physical_target.parent_path(), ec);

            std::ios_base::openmode mode = std::ios::binary | std::ios::out;
            if (append) mode |= std::ios::app;

            std::ofstream file(physical_target, mode);
            if (!file.is_open()) {
                throw events::vfs::vfs_write_error("Open for Write", virt_path, "Native file stream failed to open.");
            }

            file.write(reinterpret_cast<const char*>(data.data()), data.size());
            if (!file.good()) {
                throw events::vfs::vfs_write_error("Write Bytes", virt_path, "Native stream failed to write all bytes.");
            }
        };
    }

    void filesystem::on_list(world& ecs, const events::vfs::list_request& e) {
        std::string base_phys_path = get_physfs_path(e.virtual_path.generic_string());
        std::filesystem::path base_virt_path = e.virtual_path;
        bool recursive = e.recursive;

        e.result_command = [this, base_phys_path, base_virt_path, recursive]() -> std::vector<std::filesystem::path> {
            std::vector<std::filesystem::path> total_paths;

            auto walk_directory = [&](auto& self, const std::string& current_phys, const std::filesystem::path& current_virt) -> void {
                char** files = PHYSFS_enumerateFiles(current_phys.c_str());
                if (!files) throw_physfs_error("Directory Enumeration", current_virt);

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
        e.result_command = [this, virt_path = e.virtual_path]() -> void {
            std::filesystem::path physical_target = resolve_physical_write_path(virt_path);
            std::error_code ec;
            if (!std::filesystem::remove_all(physical_target, ec) && ec) {
                throw events::vfs::vfs_system_error("Delete File/Folder", virt_path, ec.message());
            }
        };
    }

    void filesystem::on_mkdir(world& ecs, const events::vfs::mkdir_request& e) {
        e.result_command = [this, virt_path = e.virtual_path]() -> void {
            std::filesystem::path physical_target = resolve_physical_write_path(virt_path);
            std::error_code ec;
            if (!std::filesystem::create_directories(physical_target, ec) && ec) {
                throw events::vfs::vfs_system_error("Create Directory", virt_path, ec.message());
            }
        };
    }

    void filesystem::on_rename(world& ecs, const events::vfs::rename_request& e) {
        e.result_command = [this, virt_old = e.old_virtual_path, virt_new = e.new_virtual_path]() -> void {
            std::filesystem::path physical_old = resolve_physical_write_path(virt_old);
            std::filesystem::path physical_new = resolve_physical_write_path(virt_new);

            std::error_code ec;
            std::filesystem::rename(physical_old, physical_new, ec);
            if (ec) throw events::vfs::vfs_system_error("OS Rename", physical_old, ec.message());
        };
    }

    void filesystem::on_copy(world& ecs, const events::vfs::copy_request& e) {
        e.result_command = [this, virt_src = e.source_virtual_path, virt_dest = e.destination_virtual_path]() -> void {
            std::string physfs_src = get_physfs_path(virt_src.generic_string());
            std::filesystem::path physical_new = resolve_physical_write_path(virt_dest);

            // ANTI 0-BYTE OVERWRITE GUARD: Prevents copying a file over itself!
            const char* real_dir = PHYSFS_getRealDir(physfs_src.c_str());
            if (real_dir) {
                std::filesystem::path actual_src = std::filesystem::path(real_dir) / get_sub_path(virt_src.generic_string());
                std::error_code ec;
                if (std::filesystem::exists(actual_src, ec) && std::filesystem::exists(physical_new, ec)) {
                    if (std::filesystem::equivalent(actual_src, physical_new, ec)) return; // Silent success
                }
            }

            PHYSFS_file* src_file = PHYSFS_openRead(physfs_src.c_str());
            if (!src_file) throw events::vfs::vfs_not_found_error("Copy Source Lookup", virt_src);

            std::error_code ec;
            std::filesystem::create_directories(physical_new.parent_path(), ec);

            std::ofstream dest_file(physical_new, std::ios::binary | std::ios::out);
            if (!dest_file.is_open()) {
                PHYSFS_close(src_file);
                throw events::vfs::vfs_write_error("Copy Destination Open", virt_dest, "Failed to open native write stream.");
            }

            constexpr size_t buffer_size = 4096;
            std::vector<char> buffer(buffer_size);
            PHYSFS_sint64 bytes_read = 0;

            while ((bytes_read = PHYSFS_readBytes(src_file, buffer.data(), buffer_size)) > 0) {
                dest_file.write(buffer.data(), bytes_read);
                if (!dest_file.good()) {
                    PHYSFS_close(src_file);
                    dest_file.close();
                    throw events::vfs::vfs_write_error("Copy Stream Write", virt_dest, "Disk write failure during streaming.");
                }
            }

            PHYSFS_close(src_file);
            dest_file.close();

            if (bytes_read < 0) {
                throw events::vfs::vfs_read_error("Copy Stream Read", virt_src, "Failed reading source archive bytes.");
            }
        };
    }

    void filesystem::on_move(world& ecs, const events::vfs::move_request& e) {
        // Move is architecturally identical to Rename since we only allow moving writable files natively
        events::vfs::rename_request rename_req{e.source_virtual_path, e.destination_virtual_path};
        on_rename(ecs, rename_req);
        e.result_command = std::move(rename_req.result_command);
    }

    void filesystem::on_state(world& ecs, const events::vfs::state_request& e) {
        std::string path = get_physfs_path(e.virtual_path.generic_string());

        e.result_command = [this, path, virt_path = e.virtual_path]() -> events::vfs::file_metadata {
            events::vfs::file_metadata metadata{};
            metadata.virtual_path = virt_path;

            PHYSFS_Stat stat;
            if (PHYSFS_stat(path.c_str(), &stat) == 0) {
                throw_physfs_error("State Query", virt_path);
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

    void filesystem::on_absolute(world& ecs, const events::vfs::absolute_request& e) {
        std::string physfs_path = get_physfs_path(e.virtual_path.generic_string());

        e.result_command = [this, physfs_path, virt_path = e.virtual_path]() -> std::filesystem::path {
            const char* real_dir = PHYSFS_getRealDir(physfs_path.c_str());
            if (!real_dir) throw events::vfs::vfs_not_found_error("Absolute Resolution", virt_path);

            return std::filesystem::path(real_dir) / get_sub_path(virt_path.generic_string());
        };
    }

    // MARK: - Subsystem Helpers

    std::filesystem::path filesystem::resolve_physical_write_path(const std::filesystem::path& virtual_path) const {
        std::string v_str = virtual_path.generic_string();
        std::string prefix = get_mount_prefix(v_str);

        auto it = m_writable_mounts.find(prefix);
        if (it == m_writable_mounts.end()) {
            throw events::vfs::vfs_write_error("Write Security", virtual_path, "No writable mount mapped to this path.");
        }

        std::filesystem::path root_physical = it->second;
        std::string sub_path = get_sub_path(v_str);

        std::filesystem::path raw_target = root_physical / sub_path;

        std::filesystem::path jailed_target = raw_target.lexically_normal();
        std::filesystem::path jailed_root = root_physical.lexically_normal();

        auto root_str = jailed_root.string();
        auto target_str = jailed_target.string();

        if (target_str.find(root_str) != 0) {
            throw events::vfs::vfs_system_error(
                "Security Violation",
                virtual_path,
                "Path traversal attack detected! Attempted to break out of VFS sandbox."
            );
        }

        return jailed_target;
    }

    std::string filesystem::get_mount_prefix(std::string_view v_path) const {
        size_t colon = v_path.find(':');
        if (colon == std::string_view::npos) return "";

        size_t start = v_path.find_first_not_of('/', colon + 1);
        if (start == std::string_view::npos) return "";

        size_t end = v_path.find('/', start);
        return (end == std::string_view::npos) ? std::string(v_path.substr(start))
                                               : std::string(v_path.substr(start, end - start));
    }

    std::string filesystem::get_sub_path(std::string_view v_path) const {
        size_t colon = v_path.find(':');
        if (colon == std::string_view::npos) return "";

        size_t start = v_path.find_first_not_of('/', colon + 1);
        if (start == std::string_view::npos) return "";

        size_t next_slash = v_path.find('/', start);
        if (next_slash == std::string_view::npos) return "";

        size_t sub_path_start = v_path.find_first_not_of('/', next_slash);
        if (sub_path_start == std::string_view::npos) return "";

        std::string_view p = v_path.substr(sub_path_start);
        while (!p.empty() && p.back() == '/') p.remove_suffix(1);
        return std::string(p);
    }

    std::string filesystem::get_physfs_path(std::string_view v_path) const {
        std::string prefix = get_mount_prefix(v_path);
        if (prefix.empty()) return "";
        std::string sub = get_sub_path(v_path);
        return sub.empty() ? prefix : prefix + "/" + sub;
    }

    void filesystem::throw_physfs_error(const std::string& context, const std::filesystem::path& path) const {
        PHYSFS_ErrorCode err_code = PHYSFS_getLastErrorCode();
        const char* err_desc = PHYSFS_getErrorByCode(err_code);

        switch(err_code) {
            case PHYSFS_ERR_NOT_FOUND:
                throw events::vfs::vfs_not_found_error(context, path);
            case PHYSFS_ERR_OUT_OF_MEMORY:
            case PHYSFS_ERR_NO_SPACE:
                throw events::vfs::vfs_write_error(context, path, err_desc);
            default:
                throw events::vfs::vfs_system_error(context, path, err_desc);
        }
    }

} // namespace sandbox::modules