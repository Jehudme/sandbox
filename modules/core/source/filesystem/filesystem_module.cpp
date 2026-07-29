//
// Created by jehud on 2026-06-29.
//

#include "filesystem_module.h"
#include <sandbox/sdk/filesystem.hpp>
#include <sandbox/sdk/logs.hpp>
#include <sandbox/sdk/configuration.hpp>
#include "../../../sandbox/source/core/exceptions.h"
#include "miniz.h"
#include <filesystem>
#include <algorithm>
#include <cstring>
#include <fstream>

namespace sandbox::modules {

    filesystem_t::filesystem_t(flecs::world& entity_world) : m_entity_world(entity_world) {
        sandbox::modules::logs::trace(m_entity_world, "Filesystem Module Initializing...");

        // Require configuration service
        sandbox::properties config = sandbox::modules::configuration::get_properties(m_entity_world);
        
        if (config.is_valid()) {
            if (config.has("filesystem/mounts")) {
                std::vector<std::string> mounts = config.keys("filesystem/mounts");
                sandbox::modules::logs::info(m_entity_world, "Filesystem Module: Found {} mounts in configuration.", mounts.size());
                for (const auto& mnt_name : mounts) {
                    std::string path = "filesystem/mounts/" + mnt_name;
                    auto physical = config.get<std::string>(path + "/physical");
                    auto readonly = config.get<bool>(path + "/readonly").value_or(true);
                    
                    if (physical.has_value()) {
                        // Virtual path uses URI scheme format
                        std::string virtual_path = mnt_name + "://";
                        mount(physical->c_str(), virtual_path.c_str(), readonly);
                    }
                }
            } else {
                sandbox::modules::logs::info(m_entity_world, "Filesystem Module: No 'filesystem/mounts' found in config.");
            }
        } else {
            sandbox::modules::logs::info(m_entity_world, "Filesystem Module: config object is INVALID.");
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
    std::string filesystem_t::resolve_full_physical_path(const char* virtual_path) const {
        if (!virtual_path) return "";
        std::string internal_path;
        std::string res = resolve_physical_path(virtual_path, internal_path);
        if (!res.empty()) {
            std::filesystem::path p(res);
            if (!internal_path.empty()) p /= internal_path;
            return p.string();
        }
        return "";
    }

    sandbox_file_handle_t filesystem_t::open_read(const char* virtual_path) { return {0}; }
    sandbox_file_handle_t filesystem_t::open_write(const char* virtual_path, bool append, bool force_path) { return {0}; }
    size_t filesystem_t::read(sandbox_file_handle_t handle, void* buffer, size_t bytes_to_read) { return 0; }
    size_t filesystem_t::write(sandbox_file_handle_t handle, const void* buffer, size_t bytes_to_write) { return 0; }
    bool filesystem_t::eof(sandbox_file_handle_t handle) const { return true; }
    size_t filesystem_t::tell(sandbox_file_handle_t handle) const { return 0; }
    bool filesystem_t::seek(sandbox_file_handle_t handle, size_t position) { return false; }
    size_t filesystem_t::size(sandbox_file_handle_t handle) const { return 0; }
    void filesystem_t::close(sandbox_file_handle_t handle) {}
    std::string filesystem_t::resolve_physical_path(const std::string& virtual_path, std::string& out_internal_path) const {
        std::string matched_mount;
        std::string physical_base;
        for (const auto& [mount_pt, phys_pt] : m_physical_mounts) {
            if (virtual_path.find(mount_pt) == 0) {
                if (mount_pt.length() > matched_mount.length()) {
                    matched_mount = mount_pt;
                    physical_base = phys_pt;
                }
            }
        }

        if (matched_mount.empty()) {
            return "";
        }

        out_internal_path = virtual_path.substr(matched_mount.length());
        if (!out_internal_path.empty() && out_internal_path[0] == '/') {
            out_internal_path = out_internal_path.substr(1);
        }
        
        return physical_base;
    }

    std::vector<uint8_t> filesystem_t::read_all_bytes(const char* virtual_path) {
        flecs::world world_mut = m_entity_world;
        if (!virtual_path) {
            sandbox::modules::logs::error(world_mut, "read_all_bytes failed: null virtual path");
            throw sandbox::core::filesystem_error("Null virtual path provided to read_all_bytes");
        }

        std::string internal_path;
        std::string physical_base = resolve_physical_path(virtual_path, internal_path);

        if (physical_base.empty()) {
            sandbox::modules::logs::error(world_mut, "Failed to resolve virtual path to physical mount: {}", virtual_path);
            throw sandbox::core::filesystem_error("Path resolution failed");
        }

        if (physical_base.length() >= 4 && physical_base.substr(physical_base.length() - 4) == ".zip") {
            mz_zip_archive zip_archive;
            memset(&zip_archive, 0, sizeof(zip_archive));

            if (!mz_zip_reader_init_file(&zip_archive, physical_base.c_str(), 0)) {
                sandbox::modules::logs::error(world_mut, "Failed to open zip file '{}'", physical_base);
                throw sandbox::core::filesystem_error("Failed to open zip file");
            }

            int file_index = mz_zip_reader_locate_file(&zip_archive, internal_path.c_str(), nullptr, 0);
            if (file_index < 0) {
                mz_zip_reader_end(&zip_archive);
                sandbox::modules::logs::error(world_mut, "File '{}' not found in zip archive '{}'", internal_path, physical_base);
                throw sandbox::core::filesystem_error("File not found in zip archive");
            }

            size_t uncomp_size;
            void* p = mz_zip_reader_extract_to_heap(&zip_archive, file_index, &uncomp_size, 0);
            if (!p) {
                mz_zip_reader_end(&zip_archive);
                sandbox::modules::logs::error(world_mut, "Failed to extract file '{}' from zip archive '{}'", internal_path, physical_base);
                throw sandbox::core::filesystem_error("Failed to extract file from zip");
            }

            std::vector<uint8_t> result(static_cast<uint8_t*>(p), static_cast<uint8_t*>(p) + uncomp_size);
            mz_free(p);
            mz_zip_reader_end(&zip_archive);
            sandbox::modules::logs::trace(world_mut, "Successfully read {} bytes from zip file: {}", uncomp_size, virtual_path);
            return result;
        } else {
            std::filesystem::path full_path = std::filesystem::path(physical_base) / internal_path;
            if (!std::filesystem::exists(full_path) || !std::filesystem::is_regular_file(full_path)) {
                sandbox::modules::logs::error(world_mut, "File does not exist or is not a regular file: {}", full_path.string());
                throw sandbox::core::filesystem_error("File does not exist on disk");
            }

            std::ifstream file(full_path, std::ios::binary | std::ios::ate);
            if (!file) {
                sandbox::modules::logs::error(world_mut, "Failed to open file for reading: {}", full_path.string());
                throw sandbox::core::filesystem_error("Failed to open physical file");
            }

            std::streamsize size = file.tellg();
            file.seekg(0, std::ios::beg);

            std::vector<uint8_t> buffer(size);
            if (file.read(reinterpret_cast<char*>(buffer.data()), size)) {
                sandbox::modules::logs::trace(world_mut, "Successfully read {} bytes from physical file: {}", size, virtual_path);
                return buffer;
            } else {
                sandbox::modules::logs::error(world_mut, "Failed to read data from file: {}", full_path.string());
                throw sandbox::core::filesystem_error("Failed to read from physical file");
            }
        }
    }

    bool filesystem_t::write_all_bytes(const char* virtual_path, const void* data, size_t size) {
        flecs::world world_mut = m_entity_world;
        if (!virtual_path) {
            sandbox::modules::logs::error(world_mut, "write_all_bytes failed: null virtual path");
            return false;
        }

        std::string internal_path;
        std::string physical_base = resolve_physical_path(virtual_path, internal_path);

        if (physical_base.empty()) {
            sandbox::modules::logs::error(world_mut, "Failed to resolve virtual path to physical mount: {}", virtual_path);
            return false;
        }

        if (physical_base.length() >= 4 && physical_base.substr(physical_base.length() - 4) == ".zip") {
            sandbox::modules::logs::error(world_mut, "Cannot write directly to zip archive: {}", physical_base);
            return false;
        }

        std::filesystem::path full_path = std::filesystem::path(physical_base) / internal_path;
        
        // Ensure directory exists
        if (full_path.has_parent_path()) {
            std::error_code ec;
            std::filesystem::create_directories(full_path.parent_path(), ec);
        }

        std::ofstream file(full_path, std::ios::binary | std::ios::trunc);
        if (!file) {
            sandbox::modules::logs::error(world_mut, "Failed to open file for writing: {}", full_path.string());
            return false;
        }

        file.write(reinterpret_cast<const char*>(data), size);
        if (!file.good()) {
            sandbox::modules::logs::error(world_mut, "Failed to write data to file: {}", full_path.string());
            return false;
        }
        
        sandbox::modules::logs::trace(world_mut, "Successfully wrote {} bytes to physical file: {}", size, virtual_path);
        return true;
    }
    
    std::string filesystem_t::read_all_text(const char* virtual_path) {
        std::vector<uint8_t> bytes = read_all_bytes(virtual_path);
        return std::string(bytes.begin(), bytes.end());
    }
    bool filesystem_t::write_all(const char* virtual_path, const void* data, size_t size, bool force_path) { return false; }
    bool filesystem_t::create_file(const char* virtual_path, bool force_path) { return false; }
    bool filesystem_t::remove_file(const char* virtual_path) { return false; }
    bool filesystem_t::copy(const char* source_virtual_path, const char* dest_virtual_path, bool overwrite, bool force_path) { return false; }
    bool filesystem_t::move(const char* source_virtual_path, const char* dest_virtual_path, bool overwrite, bool force_path) { return false; }
    bool filesystem_t::create_directory(const char* virtual_path, bool force_path) { return false; }
    bool filesystem_t::remove_directory(const char* virtual_path) { return false; }
    std::vector<std::string> filesystem_t::list_contents(const char* virtual_path) const { return {}; }
    bool filesystem_t::exists(const char* virtual_path) const {
        if (!virtual_path) return false;
        std::string internal_path;
        std::string physical_base = resolve_physical_path(virtual_path, internal_path);
        if (physical_base.empty()) return false;
        
        if (physical_base.length() >= 4 && physical_base.substr(physical_base.length() - 4) == ".zip") {
            mz_zip_archive zip_archive;
            memset(&zip_archive, 0, sizeof(zip_archive));
            if (!mz_zip_reader_init_file(&zip_archive, physical_base.c_str(), 0)) {
                return false;
            }
            int file_index = mz_zip_reader_locate_file(&zip_archive, internal_path.c_str(), nullptr, 0);
            mz_zip_reader_end(&zip_archive);
            return file_index >= 0;
        } else {
            std::filesystem::path full_path = std::filesystem::path(physical_base) / internal_path;
            return std::filesystem::exists(full_path);
        }
    }
    bool filesystem_t::is_file(const char* virtual_path) const { return false; }
    bool filesystem_t::is_directory(const char* virtual_path) const { return false; }
    bool filesystem_t::is_readonly(const char* virtual_path) const { return false; }
    size_t filesystem_t::file_size(const char* virtual_path) const { return 0; }
    int64_t filesystem_t::last_modified(const char* virtual_path) const { return 0; }

    std::vector<std::string> filesystem_t::list_files(const char* virtual_path, bool recursive) const {
        flecs::world world_mut = m_entity_world;
        if (!virtual_path) {
            sandbox::modules::logs::error(world_mut, "list_files failed: null virtual path");
            throw sandbox::core::filesystem_error("Null virtual path provided to list_files");
        }
        
        sandbox::modules::logs::trace(world_mut, "Listing files for virtual path: {} (recursive: {})", virtual_path, recursive);
        std::vector<std::string> results;
        std::string v_path_str = virtual_path;
        
        // Find matching mount point
        std::string matched_mount;
        std::string physical_base;
        for (const auto& [mount_pt, phys_pt] : m_physical_mounts) {
            // Prefix matching
            if (v_path_str.find(mount_pt) == 0) {
                if (mount_pt.length() > matched_mount.length()) {
                    matched_mount = mount_pt;
                    physical_base = phys_pt;
                }
            }
        }
        
        if (matched_mount.empty()) {
            sandbox::modules::logs::warn(world_mut, "No physical mount found for virtual path: {}", virtual_path);
            return results;
        }
        
        // Construct physical path
        std::string relative = v_path_str.substr(matched_mount.length());
        if (!relative.empty() && relative[0] == '/') relative = relative.substr(1);
        
        std::filesystem::path phys_target = std::filesystem::path(physical_base) / relative;
        
        try {
            if (!std::filesystem::exists(phys_target) || !std::filesystem::is_directory(phys_target)) {
                sandbox::modules::logs::warn(world_mut, "Path does not exist or is not a directory: {}", phys_target.string());
                return results;
            }
            
            if (recursive) {
                for (const auto& entry : std::filesystem::recursive_directory_iterator(phys_target)) {
                    if (entry.is_regular_file()) {
                        std::string rel_to_base = std::filesystem::relative(entry.path(), physical_base).string();
                        std::replace(rel_to_base.begin(), rel_to_base.end(), '\\', '/');
                        std::string virt = matched_mount;
                        if (virt.back() != '/') virt += '/';
                        virt += rel_to_base;
                        results.push_back(virt);
                    }
                }
            } else {
                for (const auto& entry : std::filesystem::directory_iterator(phys_target)) {
                    if (entry.is_regular_file()) {
                        std::string rel_to_base = std::filesystem::relative(entry.path(), physical_base).string();
                        std::replace(rel_to_base.begin(), rel_to_base.end(), '\\', '/');
                        std::string virt = matched_mount;
                        if (virt.back() != '/') virt += '/';
                        virt += rel_to_base;
                        results.push_back(virt);
                    }
                }
            }
        } catch (const std::exception& e) {
            sandbox::modules::logs::error(world_mut, "Filesystem error while listing files: {}", e.what());
            throw sandbox::core::filesystem_error(e.what());
        }
        
        return results;
    }

    std::vector<std::string> filesystem_t::list_directories(const char* virtual_path, bool recursive) const {
        flecs::world world_mut = m_entity_world;
        if (!virtual_path) {
            sandbox::modules::logs::error(world_mut, "list_directories failed: null virtual path");
            throw sandbox::core::filesystem_error("Null virtual path provided to list_directories");
        }
        
        sandbox::modules::logs::trace(world_mut, "Listing directories for virtual path: {} (recursive: {})", virtual_path, recursive);
        std::vector<std::string> results;
        std::string v_path_str = virtual_path;
        
        // Find matching mount point
        std::string matched_mount;
        std::string physical_base;
        for (const auto& [mount_pt, phys_pt] : m_physical_mounts) {
            // Prefix matching
            if (v_path_str.find(mount_pt) == 0) {
                if (mount_pt.length() > matched_mount.length()) {
                    matched_mount = mount_pt;
                    physical_base = phys_pt;
                }
            }
        }
        
        if (matched_mount.empty()) {
            sandbox::modules::logs::warn(world_mut, "No physical mount found for virtual path: {}", virtual_path);
            return results;
        }
        
        // Construct physical path
        std::string relative = v_path_str.substr(matched_mount.length());
        if (!relative.empty() && relative[0] == '/') relative = relative.substr(1);
        
        std::filesystem::path phys_target = std::filesystem::path(physical_base) / relative;
        
        try {
            if (!std::filesystem::exists(phys_target) || !std::filesystem::is_directory(phys_target)) {
                sandbox::modules::logs::warn(world_mut, "Path does not exist or is not a directory: {}", phys_target.string());
                return results;
            }
            
            if (recursive) {
                for (const auto& entry : std::filesystem::recursive_directory_iterator(phys_target)) {
                    if (entry.is_directory()) {
                        std::string rel_to_base = std::filesystem::relative(entry.path(), physical_base).string();
                        std::replace(rel_to_base.begin(), rel_to_base.end(), '\\', '/');
                        std::string virt = matched_mount;
                        if (virt.back() != '/') virt += '/';
                        virt += rel_to_base;
                        results.push_back(virt);
                    }
                }
            } else {
                for (const auto& entry : std::filesystem::directory_iterator(phys_target)) {
                    if (entry.is_directory()) {
                        std::string rel_to_base = std::filesystem::relative(entry.path(), physical_base).string();
                        std::replace(rel_to_base.begin(), rel_to_base.end(), '\\', '/');
                        std::string virt = matched_mount;
                        if (virt.back() != '/') virt += '/';
                        virt += rel_to_base;
                        results.push_back(virt);
                    }
                }
            }
        } catch (const std::exception& e) {
            sandbox::modules::logs::error(world_mut, "Filesystem error while listing directories: {}", e.what());
            throw sandbox::core::filesystem_error(e.what());
        }
        
        return results;
    }
}


// ==========================================
namespace sandbox::modules {
    static sandbox_requirement_info_t filesystem_reqs[] = {
        {SANDBOX_REQUIREMENT_KIND_MODULE, SANDBOX_REQUIREMENT_STRICTNESS_REQUIRED, "configuration", "sandbox", 1, 0, -1},
        {SANDBOX_REQUIREMENT_KIND_MODULE, SANDBOX_REQUIREMENT_STRICTNESS_REQUIRED, "logs", "sandbox", 1, 0, -1}
    };
    
    SANDBOX_DECLARE_MODULE(filesystem_t, {
        .name = "filesystem",
        .description = "Filesystem module",
        .architecture = "sandbox",
        .version_major = 1,
        .version_minor = 0,
        .version_patch = 0,
        .service = &sandbox_filesystem_service_t_info,
        .requirements = filesystem_reqs,
        .requirement_count = 2
    })
}
