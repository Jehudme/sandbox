#include "subsystems/filesystem/filesystem.h"
#include "sandbox/event_bus/filesystem_events.h"
#include "sandbox/event_bus/logger_events.h"
#include "sandbox/generated/schemas/filesystem_generated.h"
#include <physfs.h>
#include <fstream>

namespace sandbox::modules {

    namespace {
        /// RAII guard for PHYSFS_file handles. Ensures the handle is always
        /// closed on any exit path, including exceptions.
        struct PhysfsFileGuard {
            explicit PhysfsFileGuard(PHYSFS_file* file_handle) : m_handle(file_handle) {}
            ~PhysfsFileGuard() { if (m_handle) PHYSFS_close(m_handle); }

            // Non-copyable, movable so it can be returned from factory functions.
            PhysfsFileGuard(const PhysfsFileGuard&) = delete;
            PhysfsFileGuard& operator=(const PhysfsFileGuard&) = delete;

            [[nodiscard]] PHYSFS_file* get() const noexcept { return m_handle; }
            [[nodiscard]] bool valid() const noexcept { return m_handle != nullptr; }

        private:
            PHYSFS_file* m_handle{nullptr};
        };
    } // anonymous namespace

    filesystem_module::filesystem_module(world& ecs) {
        ecs.module<filesystem_module>("::Modules::Filesystem");
        ecs.set<sandbox::filesystem_service>({this});

        if (!PHYSFS_init(nullptr)) {
            SANDBOX_FATAL_THROW(ecs, "Failed to initialize PhysFS context layer.");
        }

        PHYSFS_permitSymbolicLinks(1);

        SANDBOX_INFO(ecs, "[Filesystem] Subsystem operational.");
    }

    filesystem_module::~filesystem_module() {
        PHYSFS_deinit();
    }

    std::expected<void, std::string> filesystem_module::mount_impl(std::string_view physical_path, std::string_view virtual_prefix, bool read_only) {
        std::string v_str = std::string(virtual_prefix);

        if (v_str.find(":/") == std::string::npos) {
            return std::unexpected("Format Validation error: virtual prefix must contain ':/'");
        }

        std::string prefix = this->get_mount_prefix(v_str);
        if (prefix.empty()) {
            return std::unexpected(std::string("Filesystem Error: ") + std::string(virtual_prefix) + " " + std::string("Mount name cannot be empty."));
        }

        if (!get_sub_path(v_str).empty()) {
            return std::unexpected(std::string("Filesystem Error: ") + std::string(virtual_prefix) + " " + std::string("Mount target cannot contain sub-directories."));
        }

        std::error_code ec;
        std::filesystem::create_directories(physical_path, ec);

        std::string phys = std::string(physical_path);

        if (!PHYSFS_mount(phys.c_str(), prefix.c_str(), read_only ? 1 : 0)) {
            return std::unexpected(get_physfs_error("Mount Operation", physical_path));
        }

        if (!read_only) {
            m_writable_mounts[prefix] = physical_path;
        }
        return {};
    }

    std::expected<void, std::string> filesystem_module::unmount_impl(std::string_view virtual_prefix) {
        std::string v_str = std::string(virtual_prefix);
        std::string prefix = this->get_mount_prefix(v_str);

        auto it = m_writable_mounts.find(prefix);
        if (it != m_writable_mounts.end()) {
            PHYSFS_unmount(it->second.string().c_str());
            m_writable_mounts.erase(it);
        }
        return {};
    }

    std::expected<std::vector<std::byte>, std::string> filesystem_module::read_impl(std::string_view virtual_path) const {
        std::string path = get_physfs_path(virtual_path);

        PhysfsFileGuard guard{PHYSFS_openRead(path.c_str())};
        if (!guard.valid()) return std::unexpected(get_physfs_error("Open for Read", virtual_path));

        PHYSFS_sint64 len = PHYSFS_fileLength(guard.get());
        std::vector<std::byte> buffer(static_cast<size_t>(len));

        if (PHYSFS_readBytes(guard.get(), buffer.data(), len) < 0) {
            return std::unexpected(get_physfs_error("Read Bytes", virtual_path));
        }

        return buffer;
    }

    std::expected<void, std::string> filesystem_module::write_impl(std::string_view virtual_path, std::vector<std::byte> data, bool append) {
        auto physical_target_res = resolve_physical_write_path(virtual_path);
        if (!physical_target_res) return std::unexpected(physical_target_res.error());
        std::filesystem::path physical_target = *physical_target_res;

        std::error_code ec;
        std::filesystem::create_directories(physical_target.parent_path(), ec);

        std::ios_base::openmode mode = std::ios::binary | std::ios::out;
        if (append) mode |= std::ios::app;

        std::ofstream file(physical_target, mode);
        if (!file.is_open()) {
            return std::unexpected(std::string("Filesystem Error: ") + std::string(virtual_path) + " " + std::string("Native file stream failed to open."));
        }

        file.write(reinterpret_cast<const char*>(data.data()), data.size());
        if (!file.good()) {
            return std::unexpected("Native stream failed to write all bytes.");
        }
        return {};
    }

    /// Recursively or flatly lists files and directories within a given virtual path.
    std::expected<std::vector<std::filesystem::path>, std::string> filesystem_module::list_impl(std::string_view virtual_path, bool recursive) const {
        std::string base_phys_path = get_physfs_path(virtual_path);
        std::filesystem::path base_virt_path = virtual_path;

        std::vector<std::filesystem::path> total_paths;

        auto walk_directory = [&](auto& self, const std::string& current_phys, const std::filesystem::path& current_virt) -> void {
            char** files = PHYSFS_enumerateFiles(current_phys.c_str());
            if (!files) return;

            for (char** file_entry = files; *file_entry != nullptr; ++file_entry) {
                std::string item_name = *file_entry;
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

    std::expected<void, std::string> filesystem_module::remove_impl(std::string_view virtual_path) {
        auto physical_target_res = resolve_physical_write_path(virtual_path);
        if (!physical_target_res) return std::unexpected(physical_target_res.error());
        std::filesystem::path physical_target = *physical_target_res;
        std::error_code ec;
        if (!std::filesystem::remove_all(physical_target, ec) && ec) {
            return std::unexpected(std::string("FS Error: ") + "Delete File/Folder " + ec.message());
        }
        return {};
    }

    std::expected<void, std::string> filesystem_module::mkdir_impl(std::string_view virtual_path) {
        auto physical_target_res = resolve_physical_write_path(virtual_path);
        if (!physical_target_res) return std::unexpected(physical_target_res.error());
        std::filesystem::path physical_target = *physical_target_res;
        std::error_code ec;
        if (!std::filesystem::create_directories(physical_target, ec) && ec) {
            return std::unexpected(std::string("FS Error: ") + "Create Directory " + ec.message());
        }
        return {};
    }

    std::expected<void, std::string> filesystem_module::rename_impl(std::string_view old_virtual_path, std::string_view new_virtual_path) {
        auto physical_old_res = resolve_physical_write_path(old_virtual_path);
        if (!physical_old_res) return std::unexpected(physical_old_res.error());
        std::filesystem::path physical_old = *physical_old_res;
        auto physical_new_res = resolve_physical_write_path(new_virtual_path);
        if (!physical_new_res) return std::unexpected(physical_new_res.error());
        std::filesystem::path physical_new = *physical_new_res;

        std::error_code ec;
        std::filesystem::rename(physical_old, physical_new, ec);
        if (ec) return std::unexpected("FS rename failed: " + ec.message());
        return {};
    }

    std::expected<void, std::string> filesystem_module::copy_impl(std::string_view source_virtual_path, std::string_view destination_virtual_path) {
        std::string physfs_src = get_physfs_path(source_virtual_path);
        auto physical_new_res = resolve_physical_write_path(destination_virtual_path);
        if (!physical_new_res) return std::unexpected(physical_new_res.error());
        std::filesystem::path physical_new = *physical_new_res;

        const char* real_dir = PHYSFS_getRealDir(physfs_src.c_str());
        if (real_dir) {
            std::filesystem::path actual_src = std::filesystem::path(real_dir) / get_sub_path(source_virtual_path);
            std::error_code ec;
            if (std::filesystem::exists(actual_src, ec) && std::filesystem::exists(physical_new, ec)) {
                if (std::filesystem::equivalent(actual_src, physical_new, ec)) return {};
            }
        }

        PhysfsFileGuard src_guard{PHYSFS_openRead(physfs_src.c_str())};
        if (!src_guard.valid()) {
            return std::unexpected(get_physfs_error("Open source for copy", source_virtual_path));
        }

        std::error_code ec;
        std::filesystem::create_directories(physical_new.parent_path(), ec);

        std::ofstream dest_file(physical_new, std::ios::binary | std::ios::out);
        if (!dest_file.is_open()) {
            return std::unexpected(
                "Failed to open destination for writing: " + physical_new.string());
        }

        constexpr size_t buffer_size = 4096;
        std::vector<char> buffer(buffer_size);
        PHYSFS_sint64 bytes_read = 0;

        while ((bytes_read = PHYSFS_readBytes(src_guard.get(), buffer.data(), buffer_size)) > 0) {
            dest_file.write(buffer.data(), bytes_read);
            if (!dest_file.good()) {
                return std::unexpected(
                    "Disk write failure while streaming to: " + physical_new.string());
            }
        }

        dest_file.close();

        if (bytes_read < 0) {
            return std::unexpected(get_physfs_error("Read during copy", source_virtual_path));
        }
        return {};
    }

    std::expected<void, std::string> filesystem_module::move_impl(std::string_view source_virtual_path, std::string_view destination_virtual_path) {
        // Propagate the rename result — discarding it was a silent strong-guarantee violation
        return rename_impl(source_virtual_path, destination_virtual_path);
    }

    std::expected<events::filesystem::file_metadata, std::string> filesystem_module::state_impl(std::string_view virtual_path) const {
        std::string path = get_physfs_path(virtual_path);

        events::filesystem::file_metadata metadata{};
        metadata.virtual_path = virtual_path;

        PHYSFS_Stat stat;
        if (PHYSFS_stat(path.c_str(), &stat) == 0) {
            return std::unexpected(get_physfs_error("State Query", virtual_path));
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

    std::expected<std::filesystem::path, std::string> filesystem_module::absolute_impl(std::string_view virtual_path) const {
        std::string physfs_path = get_physfs_path(virtual_path);

        const char* real_dir = PHYSFS_getRealDir(physfs_path.c_str());
        return std::filesystem::path(real_dir) / get_sub_path(virtual_path);
    }

    int32_t filesystem_module::mount(const char* physical_path, const char* virtual_prefix, bool read_only) {
        if (!physical_path || !virtual_prefix) return -1;
        auto res = mount_impl(physical_path, virtual_prefix, read_only);
        return res ? 0 : -1;
    }

    int32_t filesystem_module::unmount(const char* virtual_prefix) {
        if (!virtual_prefix) return -1;
        auto res = unmount_impl(virtual_prefix);
        return res ? 0 : -1;
    }

    int32_t filesystem_module::read(const char* virtual_path, sandbox_payload* out_payload) const {
        if (!virtual_path || !out_payload) return -1;
        auto res = read_impl(virtual_path);
        if (!res) return -1;
        if (res->empty()) {
            out_payload->bytes = nullptr;
            out_payload->size = 0;
            out_payload->free_func = nullptr;
            return 0;
        }
        uint8_t* ptr = static_cast<uint8_t*>(std::malloc(res->size()));
        if (!ptr) return -1;
        std::memcpy(ptr, res->data(), res->size());
        out_payload->bytes = ptr;
        out_payload->size = res->size();
        out_payload->free_func = [](void* p) { std::free(p); };
        return 0;
    }

    int32_t filesystem_module::write(const char* virtual_path, const uint8_t* data, size_t size, bool append) {
        if (!virtual_path || (!data && size > 0)) return -1;
        std::vector<std::byte> buf;
        if (size > 0) {
            buf.resize(size);
            std::memcpy(buf.data(), data, size);
        }
        auto res = write_impl(virtual_path, std::move(buf), append);
        return res ? 0 : -1;
    }

    int32_t filesystem_module::list(const char* virtual_path, bool recursive, sandbox_payload* out_payload) const {
        if (!virtual_path || !out_payload) return -1;
        auto res = list_impl(virtual_path, recursive);
        if (!res) return -1;

        flatbuffers::FlatBufferBuilder builder;
        std::vector<flatbuffers::Offset<flatbuffers::String>> fb_strings;
        for (const auto& path : *res) {
            fb_strings.push_back(builder.CreateString(path.generic_string()));
        }
        auto vector_offset = builder.CreateVector(fb_strings);
        sandbox::schemas::StringListBuilder slb(builder);
        slb.add_items(vector_offset);
        builder.Finish(slb.Finish());

        uint8_t* ptr = static_cast<uint8_t*>(std::malloc(builder.GetSize()));
        if (!ptr) return -1;
        std::memcpy(ptr, builder.GetBufferPointer(), builder.GetSize());

        out_payload->bytes = ptr;
        out_payload->size = builder.GetSize();
        out_payload->free_func = [](void* p) { std::free(p); };
        return 0;
    }

    int32_t filesystem_module::remove(const char* virtual_path) {
        if (!virtual_path) return -1;
        auto res = remove_impl(virtual_path);
        return res ? 0 : -1;
    }

    int32_t filesystem_module::mkdir(const char* virtual_path) {
        if (!virtual_path) return -1;
        auto res = mkdir_impl(virtual_path);
        return res ? 0 : -1;
    }

    int32_t filesystem_module::rename(const char* old_virtual_path, const char* new_virtual_path) {
        if (!old_virtual_path || !new_virtual_path) return -1;
        auto res = rename_impl(old_virtual_path, new_virtual_path);
        return res ? 0 : -1;
    }

    int32_t filesystem_module::copy(const char* source_virtual_path, const char* destination_virtual_path) {
        if (!source_virtual_path || !destination_virtual_path) return -1;
        auto res = copy_impl(source_virtual_path, destination_virtual_path);
        return res ? 0 : -1;
    }

    int32_t filesystem_module::move(const char* source_virtual_path, const char* destination_virtual_path) {
        if (!source_virtual_path || !destination_virtual_path) return -1;
        auto res = move_impl(source_virtual_path, destination_virtual_path);
        return res ? 0 : -1;
    }

    int32_t filesystem_module::state(const char* virtual_path, sandbox_payload* out_payload) const {
        if (!virtual_path || !out_payload) return -1;
        auto res = state_impl(virtual_path);
        if (!res) return -1;

        flatbuffers::FlatBufferBuilder builder;
        sandbox::schemas::FileType type = sandbox::schemas::FileType_Unknown;
        switch (res->type) {
            case events::filesystem::file_type::regular:   type = sandbox::schemas::FileType_Regular; break;
            case events::filesystem::file_type::directory: type = sandbox::schemas::FileType_Directory; break;
            case events::filesystem::file_type::symlink:   type = sandbox::schemas::FileType_Symlink; break;
            default: type = sandbox::schemas::FileType_Unknown; break;
        }

        auto fb_offset = sandbox::schemas::CreateFileMetadataDirect(
            builder,
            res->virtual_path.c_str(),
            res->size,
            res->creation_time,
            res->modification_time,
            res->access_time,
            type,
            res->read_only
        );
        builder.Finish(fb_offset);

        uint8_t* ptr = static_cast<uint8_t*>(std::malloc(builder.GetSize()));
        if (!ptr) return -1;
        std::memcpy(ptr, builder.GetBufferPointer(), builder.GetSize());
        
        out_payload->bytes = ptr;
        out_payload->size = builder.GetSize();
        out_payload->free_func = [](void* p) { std::free(p); };
        return 0;
    }

    int32_t filesystem_module::absolute(const char* virtual_path, sandbox_payload* out_payload) const {
        if (!virtual_path || !out_payload) return -1;
        auto res = absolute_impl(virtual_path);
        if (!res) return -1;
        std::string path_str = res->string();
        uint8_t* ptr = static_cast<uint8_t*>(std::malloc(path_str.size() + 1));
        if (!ptr) return -1;
        std::memcpy(ptr, path_str.data(), path_str.size());
        ptr[path_str.size()] = '\0';
        out_payload->bytes = ptr;
        out_payload->size = path_str.size();
        out_payload->free_func = [](void* p) { std::free(p); };
        return 0;
    }

    void filesystem_module::set_property(const char* key, const char* json_value) {
        // Currently no configuration properties needed for filesystem
    }

    int32_t filesystem_module::get_property(const char* key, sandbox_payload* out_payload) const {
        if (!out_payload) return -1;
        out_payload->bytes = nullptr;
        out_payload->size = 0;
        out_payload->free_func = nullptr;
        return 0;
    }

    /// Resolves a virtual path to a physical path and validates it to prevent path traversal attacks.
    std::expected<std::filesystem::path, std::string> filesystem_module::resolve_physical_write_path(const std::filesystem::path& virtual_path) const {
        std::string v_str = virtual_path.generic_string();
        std::string prefix = this->get_mount_prefix(v_str);

        auto it = m_writable_mounts.find(prefix);
        if (it == m_writable_mounts.end()) {
            return std::unexpected(std::string("FS Error: ") + "Write Security " + std::string(virtual_path));
        }

        std::filesystem::path root_physical = it->second;
        std::string sub_path = get_sub_path(v_str);

        std::filesystem::path raw_target = root_physical / sub_path;

        std::filesystem::path jailed_target = raw_target.lexically_normal();
        std::filesystem::path jailed_root = root_physical.lexically_normal();

        auto [root_it, target_it] = std::mismatch(jailed_root.begin(), jailed_root.end(), jailed_target.begin(), jailed_target.end());
        if (root_it != jailed_root.end()) {
            return std::unexpected(std::string("Filesystem Error: ") + std::string(virtual_path) + " " + std::string("Path traversal attack detected! Attempted to break out of VFS sandbox."));
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

    std::string filesystem_module::get_physfs_error(const std::string& context, const std::filesystem::path& path) const {
        PHYSFS_ErrorCode err_code = PHYSFS_getLastErrorCode();
        const char* err_desc = PHYSFS_getErrorByCode(err_code);

        switch(err_code) {
            case PHYSFS_ERR_NOT_FOUND:
                return std::string("Filesystem Error: ") + path.string();
            case PHYSFS_ERR_OUT_OF_MEMORY:
            case PHYSFS_ERR_NO_SPACE:
                return std::string("FS Error: ") + context + " " + path.string();
            default:
                return std::string("Filesystem Error: ") + path.string() + " " + std::string(err_desc);
        }
    }

} // namespace sandbox::modules
