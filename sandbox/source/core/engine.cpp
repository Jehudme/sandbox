#include "sandbox/core/engine.h"
#include "sandbox/core/platform.h"
#include "sandbox/utilities/filesystem.h"
#include "sandbox/event_bus/logger_events.h"
#include "sandbox/event_bus/runner_events.h"
#include "sandbox/event_bus/filesystem_events.h"
#include "sandbox/event_bus/plugin_events.h"
#include "sandbox/event_bus/event_bus.h"
#include <stdexcept>

#include "physfs.h"
#include "subsystems/logger/logger.h"
#include "subsystems/plugins/plugins.h"
#include "subsystems/runner/runner.h"
#include "subsystems/filesystem/filesystem.h"
#include "sandbox/core/plugin.h"

namespace sandbox {

    // ============================================================================
    // Helper Functions
    // ============================================================================
    namespace {

        void import_core_infrastructure(flecs::world& ecs) {
            ecs.import<modules::logger>();
            ecs.import<modules::plugins>();
            ecs.import<modules::filesystem_module>();
            ecs.import<modules::runner>();
        }

        void register_virtual_mounts(flecs::world& ecs, const std::filesystem::path& app_path) {
            const auto bin_path     = filesystem::current_path();
            const auto cache_path   = filesystem::get_user_data_directory();

            if (auto result = ecs.get<sandbox::filesystem_service>().api->mount(cache_path.generic_string(), "mount://cache", false); !result) { SANDBOX_ERROR(ecs, "Mount failed: {}", result.error()); }
            if (auto result = ecs.get<sandbox::filesystem_service>().api->mount(bin_path.generic_string(), "mount://bin", true); !result) { SANDBOX_ERROR(ecs, "Mount failed: {}", result.error()); }
            if (auto result = ecs.get<sandbox::filesystem_service>().api->mount(app_path.generic_string(), "mount://app", true); !result) { SANDBOX_ERROR(ecs, "Mount failed: {}", result.error()); }

            SANDBOX_INFO(ecs, "VFS mounts initialized (cache, bin, app).");
        }

        void build_local_modules_cache(flecs::world& ecs) {
            std::vector<filesystem::path> all_module_paths;

            try {
                auto app_modules_res = ecs.get<sandbox::filesystem_service>().api->list("mount://app/modules", false);
                if (!app_modules_res) {
                    throw std::runtime_error(app_modules_res.error());
                }
                auto app_modules = *app_modules_res;
                all_module_paths.insert(all_module_paths.end(), app_modules.begin(), app_modules.end());
            } catch (const std::exception& e) {
                SANDBOX_WARN(ecs, "Skipped app modules: {}", e.what());
            }

            try {
                auto bin_modules_res = ecs.get<sandbox::filesystem_service>().api->list("mount://bin/modules", false);
                if (!bin_modules_res) {
                    throw std::runtime_error(bin_modules_res.error());
                }
                auto bin_modules = *bin_modules_res;
                all_module_paths.insert(all_module_paths.end(), bin_modules.begin(), bin_modules.end());
            } catch (const std::exception& e) {
                SANDBOX_WARN(ecs, "Skipped bin modules: {}", e.what());
            }

            if (all_module_paths.empty()) {
                SANDBOX_INFO(ecs, "No modules found to cache.");
                return;
            }

            try {
                if (auto result = ecs.get<sandbox::filesystem_service>().api->mkdir("mount://cache/modules"); !result) { SANDBOX_ERROR(ecs, "mkdir failed: {}", result.error()); }

                for (const auto& mod_path : all_module_paths) {
                    if (mod_path.extension() == SANDBOX_COMPATIBLE_MODULE_EXTENSION) {
                        auto dest_path = std::filesystem::path("mount://cache/modules") / mod_path.filename();
                        ecs.get<sandbox::filesystem_service>().api->copy(mod_path.generic_string(), dest_path.generic_string());
                    }
                }
                SANDBOX_INFO(ecs, "Local module cache built successfully.");
            } catch (const std::exception& e) {
                SANDBOX_ERROR(ecs, "Failed building module cache: {}", e.what());
            }
        }

        void parse_and_register_manifest(flecs::world& ecs) {
            try {
                auto raw_data_res = ecs.get<sandbox::filesystem_service>().api->read("mount://app/manifest.json");
                if (!raw_data_res) {
                    SANDBOX_WARN(ecs, "read failed: {}", raw_data_res.error());
                    return;
                }
                std::vector<std::byte> raw_data = *raw_data_res;

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

                if (!module_vpath.has_extension()) {
                    module_vpath.replace_extension(SANDBOX_COMPATIBLE_MODULE_EXTENSION);
                }

                try {
                    engine_ptr->ecs.get<sandbox::plugins_service>().api->load(module_vpath.generic_string());

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
        ecs.get<sandbox::runner_service>().api->quit();
        ecs.reset();
    }

} // namespace sandbox