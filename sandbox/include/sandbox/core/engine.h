#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <cstddef>
#include <cstdint>

#include "sandbox/core/ecs.h"
#include "sandbox/utilities/properties.h"

namespace sandbox {

    class engine {
    public:
        engine();
        ~engine();

        engine(const engine&) = delete;
        engine& operator=(const engine&) = delete;

        engine(engine&&) noexcept = default;
        engine& operator=(engine&&) noexcept = default;

        void initialize(const std::filesystem::path& root_mount_path);
        void finalize();

        // Expects a Virtual File System path (e.g., "mount://core/plugins")
        void load_libraries_from_directory(const std::filesystem::path& virtual_directory_path);

        // Expects a true OS physical path resolved by the VFS scanner
        void load_library(const std::filesystem::path& physical_library_path);
        void load_module_from_library(const std::filesystem::path& physical_library_path, const char* module_name);

    private:
        // Helper routines to flatten the initialization pipeline
        void import_core_modules();
        void process_manifest_payload(std::vector<std::byte>&& data);

        // Recursive VFS directory scanner for OS-level plugin resolution
        void scan_vfs_for_plugins(const std::filesystem::path& virtual_path, uint32_t& loaded_count);

    public:
        flecs::world ecs;
    };

} // namespace sandbox