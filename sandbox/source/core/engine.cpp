#include "sandbox/core/engine.h"
#include "sandbox/core/platform.h"
#include "sandbox/utilities/filesystem.h"
#include "sandbox/utilities/config_helper.h"
#include <sandbox/modules/logger/ilogger.h>


#include "sandbox/event_bus/event_bus.h"
#include <stdexcept>

#include "physfs.h"
#include "modules/logger/logger.h"
#include "modules/runner/runner.h"
#include "modules/filesystem/filesystem.h"
#include "sandbox/core/plugin.h"
#include "sandbox/core/vfs_paths.h"
#include "sandbox/core/exceptions.h"
#include <sandbox/modules/logger/logger_api.h>
#include <sandbox/modules/filesystem/filesystem_api.h>
#include <sandbox/modules/runner/runner_api.h>
#include "utilities/loader.h"

#include "sandbox/core/environment.h"

namespace sandbox {

    namespace {

        void import_core_infrastructure(flecs::world& ecs) {
            ecs.import<modules::logger>();
            ecs.import<modules::filesystem_module>();
            ecs.import<modules::runner>();

            ecs.import<sandbox::bootstrapper>();
            sandbox::bootstrapper& boot = ecs.get_mut<sandbox::bootstrapper>();

            // Stage native engine modules into the Bootstrapper so plugins can depend on them
            // They are declared in engine_library.cpp using the standard SANDBOX_DECLARE_MODULE macro
            sandbox::detail::stage_library(ecs.c_ptr());
            
            boot.activate("core_logger");
            boot.activate("core_vfs");
            boot.activate("core_runner");
        }

        /// Mounts the essential virtual file systems required by the engine.
        /// Throws on failure — a missing mount is unrecoverable for engine startup.
        void register_virtual_mounts(flecs::world& ecs, const std::filesystem::path& app_path) {
            if (app_path.empty()) {
                throw sandbox::boot_error("[Engine] Missing 'app_mount' in configuration.");
            }
            const auto bin_path   = filesystem::current_path();
            const auto cache_path = filesystem::get_user_data_directory();
            auto fs = sandbox::sdk::filesystem(ecs);

            if (auto res = fs.mount(cache_path.generic_string(), vfs_paths::cache_mount, false); !res) {
                throw sandbox::vfs_error(std::string("[Engine] Cache mount failed. Error: ") + res.error());
            }
            if (auto res = fs.mount(bin_path.generic_string(), vfs_paths::bin_mount, true); !res) {
                throw sandbox::vfs_error(std::string("[Engine] Bin mount failed. Error: ") + res.error());
            }
            if (auto res = fs.mount(app_path.generic_string(), vfs_paths::app_mount, true); !res) {
                throw sandbox::vfs_error(std::string("[Engine] App mount failed. Error: ") + res.error());
            }

            SANDBOX_INFO(ecs, "[Engine] VFS mounts ready (cache, bin, app).");
        }

        /// Scans app and bin mounts for modules and copies them into the writable cache.
        void build_local_modules_cache(flecs::world& ecs) {
            auto fs = sandbox::sdk::filesystem(ecs);
            std::vector<std::filesystem::path> all_module_paths;

            if (auto res = fs.list("mount://app/modules", false); res) {
                for (const auto& item : *res) {
                    all_module_paths.push_back(std::filesystem::path(item));
                }
            } else {
                SANDBOX_WARN(ecs, "[Engine] No app modules found.");
            }

            if (auto res = fs.list("mount://bin/modules", false); res) {
                for (const auto& item : *res) {
                    all_module_paths.push_back(std::filesystem::path(item));
                }
            } else {
                SANDBOX_WARN(ecs, "[Engine] No bin modules found.");
            }

            if (all_module_paths.empty()) {
                SANDBOX_INFO(ecs, "[Engine] No modules discovered.");
                return;
            }

            if (auto res = fs.mkdir(vfs_paths::modules_cache_dir); !res) {
                SANDBOX_ERROR(ecs, "[Engine] Cache mkdir failed.");
            }

            for (const auto& mod_path : all_module_paths) {
                if (mod_path.extension() == SANDBOX_COMPATIBLE_MODULE_EXTENSION) {
                    auto dest = std::filesystem::path(vfs_paths::modules_cache_dir) / mod_path.filename();
                    if (auto res = fs.copy(mod_path.generic_string(), dest.generic_string()); !res) {
                        SANDBOX_WARN(ecs, "[Engine] Failed to cache '{}'", mod_path.filename().string());
                    }
                }
            }

            SANDBOX_INFO(ecs, "[Engine] Module cache built.");
        }

        /// Reads the application manifest and registers it as a global ECS entity.
        void parse_and_register_manifest(flecs::world& ecs) {
            auto fs = sandbox::sdk::filesystem(ecs);

            auto read_res = fs.read_text(vfs_paths::config_file);
            if (!read_res) {
                SANDBOX_WARN(ecs, "[Engine] manifest.json not found.");
                return;
            }

            if (read_res->empty()) {
                SANDBOX_WARN(ecs, "[Engine] manifest.json is empty.");
                return;
            }

            std::string json_content = *read_res;

            ecs.entity("::Manifest").set<properties>(properties(json_content));
            SANDBOX_INFO(ecs, "[Engine] Manifest loaded.");
        }

        /// Stages all cached libraries, activates manifest-requested modules,
        /// and executes the bootstrapper dependency resolver.
        void load_manifest_requested_plugins(flecs::world& ecs) {
            ecs.import<sandbox::bootstrapper>();
            sandbox::bootstrapper& boot = ecs.get_mut<sandbox::bootstrapper>();

            const std::string cache_modules_dir = "mount://cache/modules";
            auto fs = sandbox::sdk::filesystem(ecs);

            // Stage every compatible library found in the module cache.
            if (auto list_res = fs.list(cache_modules_dir, false); list_res) {
                for (const auto& item : *list_res) {
                    std::filesystem::path file(item);
                    if (file.extension() == SANDBOX_COMPATIBLE_MODULE_EXTENSION) {
                        auto vpath = std::filesystem::path(cache_modules_dir) / file.filename();
                        if (auto load_res = sandbox::internal::load(ecs, vpath.generic_string()); !load_res) {
                            SANDBOX_WARN(ecs, "[Engine] Failed to stage '{}': {}", file.filename().string(), load_res.error());
                        }
                    }
                }
            } else {
                SANDBOX_WARN(ecs, "[Engine] Could not list cached modules.");
            }

            // Activate modules listed in the manifest.
            auto manifest_entity = ecs.entity("::Manifest");
            if (manifest_entity.has<properties>()) {
                const auto& manifest = manifest_entity.get<properties>();
                
                std::vector<std::tuple<std::string, uint8_t, uint8_t>> modules_to_activate;

                // Try parsing as an object (map) first for versioned dependencies
                auto obj_req = manifest.get<std::map<std::string, std::string>>({"modules"});
                if (obj_req.has_value()) {
                    for (const auto& [name, version_str] : obj_req.value()) {
                        uint8_t major = 0;
                        uint8_t minor = 0;
                        auto dot_pos = version_str.find('.');
                        if (dot_pos != std::string::npos) {
                            major = static_cast<uint8_t>(std::stoi(version_str.substr(0, dot_pos)));
                            minor = static_cast<uint8_t>(std::stoi(version_str.substr(dot_pos + 1)));
                        } else if (!version_str.empty()) {
                            major = static_cast<uint8_t>(std::stoi(version_str));
                        }
                        modules_to_activate.push_back({name, major, minor});
                    }
                } else {
                    // Fall back to array of strings (legacy behavior)
                    auto arr_req = manifest.get<std::vector<std::string>>({"modules"}).value_or(std::vector<std::string>{});
                    for (const auto& name : arr_req) {
                        modules_to_activate.push_back({name, 0, 0});
                    }
                }

                if (modules_to_activate.empty()) {
                    SANDBOX_INFO(ecs, "[Engine] No modules requested in manifest.");
                } else {
                    for (const auto& [module_name, min_major, min_minor] : modules_to_activate) {
                        try {
                            boot.activate(module_name, min_major, min_minor);
                        } catch (const std::exception& e) {
                            SANDBOX_WARN(ecs, "[Engine] Manifest requested '{}' but no staged library provides it: {}", module_name, e.what());
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

    struct engine::impl {
        flecs::world ecs;
        bool initialized{false};
    };

    engine::engine(const char* json_config) : m_impl(new impl()) {
        sandbox::properties config(json_config ? json_config : "{}");
        
        m_impl->ecs.entity("::Sandbox::Environment").set<engine_environment>({config});

        import_core_infrastructure(m_impl->ecs);

        std::filesystem::path app_mount = get_config<std::string>(config, "app_mount", "");

        // Throws on mount failure — no point continuing with a broken VFS.
        register_virtual_mounts(m_impl->ecs, app_mount);
        build_local_modules_cache(m_impl->ecs);

        parse_and_register_manifest(m_impl->ecs);
        load_manifest_requested_plugins(m_impl->ecs);

        m_impl->initialized = true;
    }

    engine::~engine() {
        if (m_impl) {
            if (m_impl->initialized) {
                if (m_impl->ecs.has<sandbox::runner_service>()) {
                    auto _ = sandbox::api::quit(m_impl->ecs);
                }
                m_impl->ecs.reset();
                m_impl->initialized = false;
            }
            delete m_impl;
            m_impl = nullptr;
        }
    }

    engine::engine(engine&& other) noexcept : m_impl(other.m_impl) {
        other.m_impl = nullptr;
    }

    engine& engine::operator=(engine&& other) noexcept {
        if (this != &other) {
            if (m_impl) {
                if (m_impl->initialized) {
                    if (m_impl->ecs.has<sandbox::runner_service>()) {
                        auto _ = sandbox::api::quit(m_impl->ecs);
                    }
                    m_impl->ecs.reset();
                    m_impl->initialized = false;
                }
                delete m_impl;
            }
            m_impl = other.m_impl;
            other.m_impl = nullptr;
        }
        return *this;
    }

    void engine::run() {
        if (!m_impl || !m_impl->initialized) return;
        if (m_impl->ecs.has<sandbox::runner_service>()) {
            auto _ = sandbox::api::run_sync(m_impl->ecs);
        }
    }

    flecs::world& engine::get_ecs() {
        return m_impl->ecs;
    }

} // namespace sandbox