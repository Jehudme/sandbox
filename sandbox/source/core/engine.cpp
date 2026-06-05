#include "sandbox/core/engine.h"
#include "sandbox/core/platform.h"
#include "sandbox/utilities/filesystem.h"
#include "sandbox/event_bus/logger_events.h"
#include "sandbox/event_bus/runner_events.h"
#include "sandbox/event_bus/filesystem_events.h"
#include "sandbox/event_bus/event_bus.h"
#include <stdexcept>

#include "physfs.h"
#include "subsystems/logger/logger.h"
#include "subsystems/runner/runner.h"
#include "subsystems/filesystem/filesystem.h"
#include "sandbox/core/plugin.h"
#include "utilities/loader.h"

namespace sandbox {

    namespace {

        /// Registers fundamental core modules with the ECS world.
        void import_core_infrastructure(flecs::world& ecs) {
            ecs.import<modules::logger>();
            ecs.import<modules::filesystem_module>();
            ecs.import<modules::runner>();
        }

        /// Mounts the essential virtual file systems required by the engine.
        void register_virtual_mounts(flecs::world& ecs, const std::filesystem::path& app_path) {
            const auto bin_path     = filesystem::current_path();
            const auto cache_path   = filesystem::get_user_data_directory();

            if (auto result = ecs.get<sandbox::filesystem_service>().api->mount(cache_path.generic_string(), "mount://cache", false); !result) { SANDBOX_ERROR(ecs, "Mount failed: {}", result.error()); }
            if (auto result = ecs.get<sandbox::filesystem_service>().api->mount(bin_path.generic_string(), "mount://bin", true); !result) { SANDBOX_ERROR(ecs, "Mount failed: {}", result.error()); }
            if (auto result = ecs.get<sandbox::filesystem_service>().api->mount(app_path.generic_string(), "mount://app", true); !result) { SANDBOX_ERROR(ecs, "Mount failed: {}", result.error()); }

            SANDBOX_INFO(ecs, "VFS mounts initialized (cache, bin, app).");
        }

        /// Scans app and bin mounts for modules and populates the cache directory.
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

        /// Reads the application manifest and exposes it as a globally accessible entity.
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

        /// Processes the loaded manifest and invokes the plugin subsystem to load requested modules.
        void load_manifest_requested_plugins(engine* engine_ptr) {
            // 1. Initialize Bootstrapper
            engine_ptr->ecs.import<sandbox::bootstrapper>();

            // FIX: Grab a POINTER so we operate on the live ECS component!
            sandbox::bootstrapper& boot = engine_ptr->ecs.get_mut<sandbox::bootstrapper>();

            const std::filesystem::path cache_modules_dir = "mount://cache/modules";

            // 2. STAGE ALL LIBRARIES found in the cache
            try {
                auto cached_files_res = engine_ptr->ecs.get<sandbox::filesystem_service>().api->list(cache_modules_dir.generic_string(), false);
                if (cached_files_res) {
                    for (const auto& file : *cached_files_res) {
                        if (file.extension() == SANDBOX_COMPATIBLE_MODULE_EXTENSION) {
                            auto module_vpath = cache_modules_dir / file.filename();
                            try {
                                sandbox::internal::load(engine_ptr->ecs, module_vpath.generic_string());
                                SANDBOX_INFO(engine_ptr->ecs, "Staged library: {}", file.filename().string());
                            } catch (const std::exception& e) {
                                SANDBOX_WARN(engine_ptr->ecs, "Failed to load library '{}': {}", file.filename().string(), e.what());
                            }
                        }
                    }
                }
            } catch (const std::exception& e) {
                SANDBOX_ERROR(engine_ptr->ecs, "Could not list cached modules: {}", e.what());
            }

            // 3. ACTIVATE REQUESTED MODULES FROM MANIFEST
            auto manifest_entity = engine_ptr->ecs.entity("::Manifest");
            if (manifest_entity.has<properties>()) {
                const auto& manifest = manifest_entity.get<properties>();
                auto requested_modules = manifest.get<std::vector<std::string>>({"modules"}).value_or(std::vector<std::string>{});

                if (requested_modules.empty()) {
                    SANDBOX_INFO(engine_ptr->ecs, "No modules requested in manifest.");
                } else {
                    for (const auto& module_name : requested_modules) {
                        // FIX: Use arrow operator
                        boot.activate(module_name);
                    }
                }
            } else {
                SANDBOX_WARN(engine_ptr->ecs, "Manifest properties missing. Skipping plugin activation.");
            }

            // 4. EXECUTE BOOTSTRAPPER (Dependency Resolution & Boot)
            // FIX: Use arrow operator
            boot.execute(engine_ptr->ecs);
        }

    }

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

        // Let the new architecture do its magic!
        load_manifest_requested_plugins(this);
    }

    void engine::finalize() {
        ecs.get<sandbox::runner_service>().api->quit();
        ecs.reset();
    }

} // namespace sandbox