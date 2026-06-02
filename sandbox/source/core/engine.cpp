#include "sandbox/core/engine.h"
#include "sandbox/core/platform.h"
#include "sandbox/utilities/filesystem.h"
#include "sandbox/macros/logger.h"
#include "sandbox/macros/runner.h"
#include "sandbox/macros/vfs.h"
#include "sandbox/utilities/events.h"
#include <stdexcept>

#include "physfs.h"
#include "modules/logger.h"
#include "modules/plugins.h"
#include "modules/runner.h"
#include "modules/vfs.h"
#include "sandbox/core/arguments.h"
#include "sandbox/core/plugin.h"
#include "sandbox/macros/plugins.h"

namespace sandbox {

    // ============================================================================
    // Helper Functions
    // ============================================================================
    namespace {

        void import_core_infrastructure(flecs::world& ecs) {
            ecs.import<modules::logger>();
            ecs.import<modules::plugins>();
            ecs.import<modules::filesystem>();
            ecs.import<modules::runner>();
        }

        void register_virtual_mounts(flecs::world& ecs, const std::filesystem::path& app_path) {
            const auto bin_path = filesystem::current_path();
            const auto cache_path = filesystem::get_user_data_directory();

            SANDBOX_VFS_MOUNT(ecs, cache_path, "mount://cache", false);
            SANDBOX_VFS_MOUNT(ecs, bin_path, "mount://bin", true);
            SANDBOX_VFS_MOUNT(ecs, app_path, "mount://app", true);

            SANDBOX_INFO(ecs, "VFS mounts initialized (cache, bin, app).");
        }

        void build_local_modules_cache(flecs::world& ecs) {
            std::vector<filesystem::path> all_module_paths;

            // Safely gather app modules
            try {
                auto app_modules = SANDBOX_VFS_EXEC_LIST(ecs, "mount://app/modules", false);
                all_module_paths.insert(all_module_paths.end(), app_modules.begin(), app_modules.end());
            } catch (const std::exception& e) {
                SANDBOX_WARN(ecs, "Skipped app modules: {}", e.what());
            }

            // Safely gather bin modules
            try {
                auto bin_modules = SANDBOX_VFS_EXEC_LIST(ecs, "mount://bin/modules", false);
                all_module_paths.insert(all_module_paths.end(), bin_modules.begin(), bin_modules.end());
            } catch (const std::exception& e) {
                SANDBOX_WARN(ecs, "Skipped bin modules: {}", e.what());
            }

            if (all_module_paths.empty()) {
                SANDBOX_INFO(ecs, "No modules found to cache.");
                return;
            }

            try {
                SANDBOX_VFS_EXEC_MKDIR(ecs, "mount://cache/modules");

                for (const auto& mod_path : all_module_paths) {
                    if (mod_path.extension() == SANDBOX_COMPATIBLE_MODULE_EXTENSION) {
                        auto dest_path = std::filesystem::path("mount://cache/modules") / mod_path.filename();
                        SANDBOX_VFS_EXEC_COPY(ecs, mod_path, dest_path);
                    }
                }
                SANDBOX_INFO(ecs, "Local module cache built successfully.");
            } catch (const std::exception& e) {
                SANDBOX_ERROR(ecs, "Failed building module cache: {}", e.what());
            }
        }

        void parse_and_register_manifest(flecs::world& ecs) {
            try {
                std::vector<std::byte> raw_data = SANDBOX_VFS_EXEC_READ(ecs, "mount://app/manifest.json");

                if (raw_data.empty()) {
                    SANDBOX_WARN(ecs, "manifest.json is empty.");
                    return;
                }

                std::string json_content(reinterpret_cast<const char*>(raw_data.data()), raw_data.size());
                ecs.entity("::Manifest").set<properties>(properties(json_content));

                SANDBOX_INFO(ecs, "Manifest loaded and registered.");
            } catch (const std::exception& e) {
                SANDBOX_WARN(ecs, "Could not load manifest.json: {}", e.what());
            }
        }

        void load_manifest_requested_plugins(engine* engine_ptr) {
            auto manifest_entity = engine_ptr->ecs.entity("::Manifest");
            if (!manifest_entity.has<properties>()) {
                SANDBOX_WARN(engine_ptr->ecs, "Manifest properties missing. Skipping plugin load.");
                return;
            }

            const auto& manifest = manifest_entity.get<properties>();
            const std::filesystem::path cache_modules_dir = "mount://cache/modules";

            auto requested_modules = manifest.get<std::vector<std::string>>({"modules"}).value_or(std::vector<std::string>{});

            if (requested_modules.empty()) {
                SANDBOX_INFO(engine_ptr->ecs, "No modules requested in manifest.");
                return;
            }

            for (const auto& module_name : requested_modules) {
                auto module_vpath = cache_modules_dir / module_name;

                // Automatically append compatible extension (.so/.dll/.dylib) if omitted by modder
                if (!module_vpath.has_extension()) {
                    module_vpath.replace_extension(SANDBOX_COMPATIBLE_MODULE_EXTENSION);
                }

                try {
                    SANDBOX_PLUGIN_LOAD(engine_ptr->ecs, module_vpath);
                    SANDBOX_INFO(engine_ptr->ecs, "Loaded plugin: {}", module_name);
                } catch (const std::exception& e) {
                    SANDBOX_ERROR(engine_ptr->ecs, "Failed to load plugin '{}': {}", module_name, e.what());
                }
            }
        }

    }

    // ============================================================================
    // Class Implementation
    // ============================================================================

    engine::engine() = default;

    engine::~engine() {
        finalize();
    }

    void engine::initialize(const arguments& args) {
        sandbox::configure_plugin_os_api();
        ecs.entity("::Sandbox::Arguments").set(args);

        import_core_infrastructure(ecs);

        register_virtual_mounts(ecs, args.app_mount);
        build_local_modules_cache(ecs);

        parse_and_register_manifest(ecs);
        load_manifest_requested_plugins(this);
    }

    void engine::finalize() {
        ecs.reset();
    }

} // namespace sandbox