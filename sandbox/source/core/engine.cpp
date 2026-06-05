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
        /// Throws on failure — a missing mount is unrecoverable for engine startup.
        void register_virtual_mounts(flecs::world& ecs, const std::filesystem::path& app_path) {
            const auto bin_path   = filesystem::current_path();
            const auto cache_path = filesystem::get_user_data_directory();
            const auto& fs        = ecs.get<sandbox::filesystem_service>().api;

            if (auto res = fs->mount(cache_path.generic_string(), "mount://cache", false); !res) {
                throw std::runtime_error("[Engine] Cache mount failed: " + res.error());
            }
            if (auto res = fs->mount(bin_path.generic_string(), "mount://bin", true); !res) {
                throw std::runtime_error("[Engine] Bin mount failed: " + res.error());
            }
            if (auto res = fs->mount(app_path.generic_string(), "mount://app", true); !res) {
                throw std::runtime_error("[Engine] App mount failed: " + res.error());
            }

            SANDBOX_INFO(ecs, "[Engine] VFS mounts ready (cache, bin, app).");
        }

        /// Scans app and bin mounts for modules and copies them into the writable cache.
        void build_local_modules_cache(flecs::world& ecs) {
            const auto& fs = ecs.get<sandbox::filesystem_service>().api;
            std::vector<std::filesystem::path> all_module_paths;

            if (auto res = fs->list("mount://app/modules", false); res) {
                all_module_paths.insert(all_module_paths.end(), res->begin(), res->end());
            } else {
                SANDBOX_WARN(ecs, "[Engine] No app modules found: {}", res.error());
            }

            if (auto res = fs->list("mount://bin/modules", false); res) {
                all_module_paths.insert(all_module_paths.end(), res->begin(), res->end());
            } else {
                SANDBOX_WARN(ecs, "[Engine] No bin modules found: {}", res.error());
            }

            if (all_module_paths.empty()) {
                SANDBOX_INFO(ecs, "[Engine] No modules discovered.");
                return;
            }

            if (auto res = fs->mkdir("mount://cache/modules"); !res) {
                SANDBOX_ERROR(ecs, "[Engine] Cache mkdir failed: {}", res.error());
            }

            for (const auto& mod_path : all_module_paths) {
                if (mod_path.extension() == SANDBOX_COMPATIBLE_MODULE_EXTENSION) {
                    auto dest = std::filesystem::path("mount://cache/modules") / mod_path.filename();
                    if (auto res = fs->copy(mod_path.generic_string(), dest.generic_string()); !res) {
                        SANDBOX_WARN(ecs, "[Engine] Failed to cache '{}': {}", mod_path.filename().string(), res.error());
                    }
                }
            }

            SANDBOX_INFO(ecs, "[Engine] Module cache built.");
        }

        /// Reads the application manifest and registers it as a global ECS entity.
        void parse_and_register_manifest(flecs::world& ecs) {
            const auto& fs = ecs.get<sandbox::filesystem_service>().api;

            auto raw_data_res = fs->read("mount://app/manifest.json");
            if (!raw_data_res) {
                SANDBOX_WARN(ecs, "[Engine] manifest.json not found: {}", raw_data_res.error());
                return;
            }

            if (raw_data_res->empty()) {
                SANDBOX_WARN(ecs, "[Engine] manifest.json is empty.");
                return;
            }

            std::string json_content(
                reinterpret_cast<const char*>(raw_data_res->data()),
                raw_data_res->size());

            ecs.entity("::Manifest").set<properties>(properties(json_content));
            SANDBOX_INFO(ecs, "[Engine] Manifest loaded.");
        }

        /// Stages all cached libraries, activates manifest-requested modules,
        /// and executes the bootstrapper dependency resolver.
        void load_manifest_requested_plugins(flecs::world& ecs) {
            ecs.import<sandbox::bootstrapper>();
            sandbox::bootstrapper& boot = ecs.get_mut<sandbox::bootstrapper>();

            const std::string cache_modules_dir = "mount://cache/modules";
            const auto& fs = ecs.get<sandbox::filesystem_service>().api;

            // Stage every compatible library found in the module cache.
            if (auto cached_res = fs->list(cache_modules_dir, false); cached_res) {
                for (const auto& file : *cached_res) {
                    if (file.extension() == SANDBOX_COMPATIBLE_MODULE_EXTENSION) {
                        auto vpath = std::filesystem::path(cache_modules_dir) / file.filename();
                        if (auto load_res = sandbox::internal::load(ecs, vpath.generic_string()); !load_res) {
                            SANDBOX_WARN(ecs, "[Engine] Failed to stage '{}': {}", file.filename().string(), load_res.error());
                        }
                    }
                }
            } else {
                SANDBOX_WARN(ecs, "[Engine] Could not list cached modules: {}", cached_res.error());
            }

            // Activate modules listed in the manifest.
            auto manifest_entity = ecs.entity("::Manifest");
            if (manifest_entity.has<properties>()) {
                const auto& manifest = manifest_entity.get<properties>();
                auto requested = manifest.get<std::vector<std::string>>({"modules"})
                                        .value_or(std::vector<std::string>{});

                if (requested.empty()) {
                    SANDBOX_INFO(ecs, "[Engine] No modules requested in manifest.");
                } else {
                    for (const auto& module_name : requested) {
                        if (!boot.activate(module_name)) {
                            SANDBOX_WARN(ecs, "[Engine] Manifest requested '{}' but no staged library provides it.", module_name);
                        }
                    }
                }
            } else {
                SANDBOX_WARN(ecs, "[Engine] No manifest found — skipping plugin activation.");
            }

            // Resolve dependency graph and boot all activated modules.
            boot.execute(ecs);
        }

    } // anonymous namespace

    engine::engine() = default;

    engine::~engine() {
        // Guard against double-finalize and against calling finalize when initialize()
        // was never invoked (which would leave runner_service null).
        if (m_initialized) {
            finalize();
        }
    }

    void engine::initialize(const arguments& args) {
        // Note: configure_plugin_os_api() must be called by the host (e.g. launcher)
        // BEFORE constructing the engine, as the flecs::world member is created
        // at engine construction time, before initialize() is ever called.
        ecs.entity("::Sandbox::Arguments").set(args);

        import_core_infrastructure(ecs);

        // Throws on mount failure — no point continuing with a broken VFS.
        register_virtual_mounts(ecs, args.app_mount);
        build_local_modules_cache(ecs);

        parse_and_register_manifest(ecs);
        load_manifest_requested_plugins(ecs);

        m_initialized = true;
    }

    void engine::finalize() {
        if (!m_initialized) return;
        m_initialized = false;

        if (ecs.has<sandbox::runner_service>()) {
            if (auto* runner = ecs.get<sandbox::runner_service>().api; runner != nullptr) {
                runner->quit();
            }
        }
        ecs.reset();
    }

} // namespace sandbox