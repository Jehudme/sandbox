#include "subsystems/filesystem/filesystem.h"
#include "sandbox/event_bus/filesystem_events.h"
#include "sandbox/event_bus/logger_events.h"
#include <physfs.h>
#include <fstream>

namespace sandbox::modules {

    filesystem_module::filesystem_module(world& ecs) {
        ecs.module<filesystem_module>("::Modules::Filesystem");
        ecs.set<sandbox::filesystem_service>({this});

        if (!PHYSFS_init(nullptr)) {
            SANDBOX_FATAL_THROW(ecs, "[Filesystem] Failed to initialize PhysFS context layer.");
        }

        PHYSFS_permitSymbolicLinks(1);

        SANDBOX_INFO(ecs, "[Filesystem] Functional command factory subsystem operational.");
    }

    filesystem_module::~filesystem_module() {
        PHYSFS_deinit();
    }

    void filesystem_module::mount(std::string_view physical_path, std::string_view virtual_prefix, bool read_only) {
        std::string v_str = std::string(virtual_prefix);

        if (v_str.find(":/") == std::string::npos) {
            throw events::filesystem::filesystem_mount_error("Format Validation", virtual_prefix, "Missing protocol separator (e.g., 'mount://').");
        }

        std::string prefix = this->get_mount_prefix(v_str);
        if (prefix.empty()) {
            throw events::filesystem::filesystem_mount_error("Format Validation", virtual_prefix, "Mount name cannot be empty.");
        }

        if (!get_sub_path(v_str).empty()) {
            throw events::filesystem::filesystem_mount_error("Format Validation", virtual_prefix, "Mount target cannot contain sub-directories.");
        }

        std::error_code ec;
        std::filesystem::create_directories(physical_path, ec);

        std::string phys = std::string(physical_path);

        if (!PHYSFS_mount(phys.c_str(), prefix.c_str(), read_only ? 1 : 0)) {
            throw_physfs_error("Mount Operation", physical_path);
        }

        if (!read_only) {
            m_writable_mounts[prefix] = physical_path;
        }
    }

    void filesystem_module::unmount(std::string_view virtual_prefix) {
        std::string v_str = std::string(virtual_prefix);
        std::string prefix = this->get_mount_prefix(v_str);

        auto it = m_writable_mounts.find(prefix);
        if (it != m_writable_mounts.end()) {
            PHYSFS_unmount(it->second.string().c_str());
            m_writable_mounts.erase(it);
        }
    }

    std::vector<std::byte> filesystem_module::read(std::string_view virtual_path) {
        std::string path = get_physfs_path(virtual_path);

        PHYSFS_file* file = PHYSFS_openRead(path.c_str());
        if (!file) throw_physfs_error("Open for Read", virtual_path);

        PHYSFS_sint64 len = PHYSFS_fileLength(file);
        std::vector<std::byte> buffer(static_cast<size_t>(len));

        if (PHYSFS_readBytes(file, buffer.data(), len) < 0) {
            PHYSFS_close(file);
            throw_physfs_error("Read Bytes", virtual_path);
        }

        PHYSFS_close(file);
        return buffer;
    }

    void filesystem_module::write(std::string_view virtual_path, std::vector<std::byte> data, bool append) {
        std::filesystem::path physical_target = resolve_physical_write_path(virtual_path);

        std::error_code ec;
        std::filesystem::create_directories(physical_target.parent_path(), ec);

        std::ios_base::openmode mode = std::ios::binary | std::ios::out;
        if (append) mode |= std::ios::app;

        std::ofstream file(physical_target, mode);
        if (!file.is_open()) {
            throw events::filesystem::filesystem_write_error("Open for Write", virtual_path, "Native file stream failed to open.");
        }

        file.write(reinterpret_cast<const char*>(data.data()), data.size());
        if (!file.good()) {
            throw events::filesystem::filesystem_write_error("Write Bytes", virtual_path, "Native stream failed to write all bytes.");
        }
    }

    std::vector<std::filesystem::path> filesystem_module::list(std::string_view virtual_path, bool recursive) {
        std::string base_phys_path = get_physfs_path(virtual_path);
        std::filesystem::path base_virt_path = virtual_path;

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
    }

    void filesystem_module::remove(std::string_view virtual_path) {
        std::filesystem::path physical_target = resolve_physical_write_path(virtual_path);
        std::error_code ec;
        if (!std::filesystem::remove_all(physical_target, ec) && ec) {
            throw events::filesystem::filesystem_system_error("Delete File/Folder", virtual_path, ec.message());
        }
    }

    void filesystem_module::mkdir(std::string_view virtual_path) {
        std::filesystem::path physical_target = resolve_physical_write_path(virtual_path);
        std::error_code ec;
        if (!std::filesystem::create_directories(physical_target, ec) && ec) {
            throw events::filesystem::filesystem_system_error("Create Directory", virtual_path, ec.message());
        }
    }

    void filesystem_module::rename(std::string_view old_virtual_path, std::string_view new_virtual_path) {
        std::filesystem::path physical_old = resolve_physical_write_path(old_virtual_path);
        std::filesystem::path physical_new = resolve_physical_write_path(new_virtual_path);

        std::error_code ec;
        std::filesystem::rename(physical_old, physical_new, ec);
        if (ec) throw events::filesystem::filesystem_system_error("OS Rename", physical_old, ec.message());
    }

    void filesystem_module::copy(std::string_view source_virtual_path, std::string_view destination_virtual_path) {
        std::string physfs_src = get_physfs_path(source_virtual_path);
        std::filesystem::path physical_new = resolve_physical_write_path(destination_virtual_path);

        const char* real_dir = PHYSFS_getRealDir(physfs_src.c_str());
        if (real_dir) {
            std::filesystem::path actual_src = std::filesystem::path(real_dir) / get_sub_path(source_virtual_path);
            std::error_code ec;
            if (std::filesystem::exists(actual_src, ec) && std::filesystem::exists(physical_new, ec)) {
                if (std::filesystem::equivalent(actual_src, physical_new, ec)) return; // Silent success
            }
        }

        PHYSFS_file* src_file = PHYSFS_openRead(physfs_src.c_str());
        if (!src_file) throw events::filesystem::filesystem_not_found_error("Copy Source Lookup", source_virtual_path);

        std::error_code ec;
        std::filesystem::create_directories(physical_new.parent_path(), ec);

        std::ofstream dest_file(physical_new, std::ios::binary | std::ios::out);
        if (!dest_file.is_open()) {
            PHYSFS_close(src_file);
            throw events::filesystem::filesystem_write_error("Copy Destination Open", destination_virtual_path, "Failed to open native write stream.");
        }

        constexpr size_t buffer_size = 4096;
        std::vector<char> buffer(buffer_size);
        PHYSFS_sint64 bytes_read = 0;

        while ((bytes_read = PHYSFS_readBytes(src_file, buffer.data(), buffer_size)) > 0) {
            dest_file.write(buffer.data(), bytes_read);
            if (!dest_file.good()) {
                PHYSFS_close(src_file);
                dest_file.close();
                throw events::filesystem::filesystem_write_error("Copy Stream Write", destination_virtual_path, "Disk write failure during streaming.");
            }
        }

        PHYSFS_close(src_file);
        dest_file.close();

        if (bytes_read < 0) {
            throw events::filesystem::filesystem_read_error("Copy Stream Read", source_virtual_path, "Failed reading source archive bytes.");
        }
    }

    void filesystem_module::move(std::string_view source_virtual_path, std::string_view destination_virtual_path) {
        rename(source_virtual_path, destination_virtual_path);
    }

    events::filesystem::file_metadata filesystem_module::state(std::string_view virtual_path) {
        std::string path = get_physfs_path(virtual_path);

        events::filesystem::file_metadata metadata{};
        metadata.virtual_path = virtual_path;

        PHYSFS_Stat stat;
        if (PHYSFS_stat(path.c_str(), &stat) == 0) {
            throw_physfs_error("State Query", virtual_path);
        }

        metadata.size = stat.filesize;
        metadata.creation_time = stat.createtime;
        metadata.modification_time = stat.modtime;
        metadata.access_time = stat.accesstime;
        metadata.read_only = (stat.readonly != 0);

        switch (stat.filetype) {
            case PHYSFS_FILETYPE_REGULAR:   metadata.type = events::filesystem::file_type::regular;   break;
            case PHYSFS_FILETYPE_DIRECTORY: metadata.type = events::filesystem::file_type::directory; break;
            case PHYSFS_FILETYPE_SYMLINK:   metadata.type = events::filesystem::file_type::symlink;   break;
            default:                        metadata.type = events::filesystem::file_type::unknown;   break;
        }

        return metadata;
    }

    std::filesystem::path filesystem_module::absolute(std::string_view virtual_path) {
        std::string physfs_path = get_physfs_path(virtual_path);

        const char* real_dir = PHYSFS_getRealDir(physfs_path.c_str());
        if (!real_dir) throw events::filesystem::filesystem_not_found_error("Absolute Resolution", virtual_path);

        return std::filesystem::path(real_dir) / get_sub_path(virtual_path);
    }

    std::filesystem::path filesystem_module::resolve_physical_write_path(const std::filesystem::path& virtual_path) const {
        std::string v_str = virtual_path.generic_string();
        std::string prefix = this->get_mount_prefix(v_str);

        auto it = m_writable_mounts.find(prefix);
        if (it == m_writable_mounts.end()) {
            throw events::filesystem::filesystem_write_error("Write Security", virtual_path, "No writable mount mapped to this path.");
        }

        std::filesystem::path root_physical = it->second;
        std::string sub_path = get_sub_path(v_str);

        std::filesystem::path raw_target = root_physical / sub_path;

        std::filesystem::path jailed_target = raw_target.lexically_normal();
        std::filesystem::path jailed_root = root_physical.lexically_normal();

        auto root_str = jailed_root.string();
        auto target_str = jailed_target.string();

        if (target_str.find(root_str) != 0) {
            throw events::filesystem::filesystem_system_error(
                "Security Violation",
                virtual_path,
                "Path traversal attack detected! Attempted to break out of VFS sandbox."
            );
        }

        return jailed_target;
    }

    std::string filesystem_module::get_mount_prefix(std::string_view v_path) const {
        size_t colon = v_path.find(':');
        if (colon == std::string_view::npos) return "";

        size_t start = v_path.find_first_not_of('/', colon + 1);
        if (start == std::string_view::npos) return "";

        size_t end = v_path.find('/', start);
        return (end == std::string_view::npos) ? std::string(v_path.substr(start))
                                               : std::string(v_path.substr(start, end - start));
    }

    std::string filesystem_module::get_sub_path(std::string_view v_path) const {
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

    std::string filesystem_module::get_physfs_path(std::string_view v_path) const {
        std::string prefix = this->get_mount_prefix(v_path);
        if (prefix.empty()) return "";
        std::string sub = get_sub_path(v_path);
        return sub.empty() ? prefix : prefix + "/" + sub;
    }

    void filesystem_module::throw_physfs_error(const std::string& context, const std::filesystem::path& path) const {
        PHYSFS_ErrorCode err_code = PHYSFS_getLastErrorCode();
        const char* err_desc = PHYSFS_getErrorByCode(err_code);

        switch(err_code) {
            case PHYSFS_ERR_NOT_FOUND:
                throw events::filesystem::filesystem_not_found_error(context, path);
            case PHYSFS_ERR_OUT_OF_MEMORY:
            case PHYSFS_ERR_NO_SPACE:
                throw events::filesystem::filesystem_write_error(context, path, err_desc);
            default:
                throw events::filesystem::filesystem_system_error(context, path, err_desc);
        }
    }

} // namespace sandbox::modules
