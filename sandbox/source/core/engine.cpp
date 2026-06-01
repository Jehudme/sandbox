#include "sandbox/core/engine.h"
#include "sandbox/core/platform.h"
#include "sandbox/utilities/filesystem.h"
#include "sandbox/macros/logger.h"
#include "sandbox/macros/runner.h"
#include "sandbox/macros/vfs.h"
#include <stdexcept>

#include "physfs.h"
#include "modules/logger.h"
#include "modules/runner.h"
#include "modules/vfs.h"
#include "sandbox/core/plugin.h"

namespace sandbox {

    // ============================================================================
    // 1. Anonymous Helper Functions
    // ============================================================================
    namespace {

        void import_core_infrastructure(flecs::world& ecs) {
            ecs.import<modules::filesystem>();
            ecs.import<modules::logger>();
            ecs.import<modules::runner>();
        }

        void register_virtual_mounts(flecs::world& ecs, const std::filesystem::path& application_path) {
            std::filesystem::path executable_path = filesystem::current_path();
            std::filesystem::path user_path = filesystem::get_user_data_directory();

            SANDBOX_VFS_MOUNT(ecs, user_path, "mount://user", false);
            SANDBOX_VFS_MOUNT(ecs, executable_path, "mount://executable", true);
            SANDBOX_VFS_MOUNT(ecs, application_path, "mount://application", true);
        }

        void build_local_modules_cache(flecs::world& ecs) {
            std::vector<filesystem::path> discoverable_paths = {};

            // Safeguard path lists to prevent crashing if module paths are absent
            try {
                auto app_mods = SANDBOX_VFS_EXEC_LIST(ecs, "mount://application/modules", false);
                discoverable_paths.insert(discoverable_paths.end(), app_mods.begin(), app_mods.end());
            } catch (...) {}

            try {
                auto exe_mods = SANDBOX_VFS_EXEC_LIST(ecs, "mount://executable/modules", false);
                discoverable_paths.insert(discoverable_paths.end(), exe_mods.begin(), exe_mods.end());
            } catch (...) {}

            if (discoverable_paths.empty()) return;
            SANDBOX_VFS_EXEC_MKDIR(ecs, "mount://user/modules");

            for (const auto& path : discoverable_paths) {
                if (path.extension() == SANDBOX_COMPATIBLE_MODULE_EXTENSION) {
                    std::filesystem::path target_destination = std::filesystem::path("mount://user/modules") / path.filename();
                    SANDBOX_VFS_EXEC_COPY(ecs, path, target_destination);
                }
            }
        }

        void parse_and_register_manifest(flecs::world& ecs) {
            std::vector<std::byte> data = SANDBOX_VFS_EXEC_READ(ecs, "mount://application/manifest.json");

            if (data.empty()) {
                SANDBOX_WARN(ecs, "[Engine] manifest.json was empty or could not be loaded.");
                return;
            }

            std::string json_string(reinterpret_cast<const char*>(data.data()), data.size());
            ecs.entity("::Manifest").set<properties>(properties(json_string));
        }

        void load_manifest_requested_plugins(engine* engine_instance) {
            auto manifest_entity = engine_instance->ecs.entity("::Manifest");
            if (!manifest_entity.has<properties>()) return;

            const properties& manifest = manifest_entity.get<properties>();
            std::filesystem::path local_cache_root = "mount://user/modules";

            auto modules_list = manifest.get<std::vector<std::string>>({"modules"}).value_or(std::vector<std::string>{});

            for (const auto& module_name : modules_list) {
                std::filesystem::path module_virtual_path = local_cache_root / module_name;

                // Automatically append compatible extension (.so/.dll) if omitted by modder
                if (!module_virtual_path.has_extension()) {
                    module_virtual_path.replace_extension(SANDBOX_COMPATIBLE_MODULE_EXTENSION);
                }

                try {
                    engine_instance->load_library(module_virtual_path);
                } catch (const std::exception& error) {
                    SANDBOX_ERROR(engine_instance->ecs, "[Engine] Failed to link manifest module '{}': {}", module_name, error.what());
                }
            }
        }

    } // namespace



    // ============================================================================
    // 2. Class Implementation
    // ============================================================================

    engine::engine() = default;

    engine::~engine() {
        finalize();
    }

    void engine::initialize(const std::filesystem::path& application_path) {
        sandbox::configure_plugin_os_api();

        import_core_infrastructure(ecs);

        register_virtual_mounts(ecs, application_path);
        build_local_modules_cache(ecs);

        parse_and_register_manifest(ecs);
        load_manifest_requested_plugins(this);
    }

    void engine::finalize() {
        ecs.reset();
    }

    void engine::load_library(const std::filesystem::path& virtual_path) {
        std::filesystem::path physical_path = SANDBOX_VFS_EXEC_ABSOLUTE(ecs, virtual_path);

        if (physical_path.empty()) {
            SANDBOX_ERROR_THROW(ecs, "Unresolved virtual file system path: {}", virtual_path.string());
        }

        static const std::string entry_point = "SandboxLibraryMain";
        load_module_from_library(physical_path, entry_point.c_str());
    }

    void engine::load_module_from_library(const std::filesystem::path& physical_library_path, std::string_view module_name) {
        std::string clean_filename = filesystem::strip_extension(physical_library_path).string();
        SANDBOX_DEBUG(ecs, "[Engine] Linking: {}::{}", physical_library_path.filename().string(), module_name);

        auto library = ecs_import_from_library(ecs.c_ptr(), clean_filename.c_str(), module_name.data());

        if (library) {
            SANDBOX_INFO(ecs, "[Engine] Mounted: {}", physical_library_path.filename().string());
        } else {
            SANDBOX_ERROR(ecs, "[Engine] Mount failed: {}", physical_library_path.filename().string());
        }
    }

} // namespace sandbox