#include "sandbox/core/engine.h"
#include "sandbox/core/platform.h"
#include "sandbox/utilities/filesystem.h"
#include "sandbox/utilities/config_helper.h"
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

#include "sandbox/core/environment.h"

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
            if (app_path.empty()) {
                throw std::runtime_error("[Engine] Missing 'app_mount' in configuration.");
            }
            const auto bin_path   = filesystem::current_path();
            const auto cache_path = filesystem::get_user_data_directory();
            const auto& fs        = ecs.get<sandbox::filesystem_service>().api;

            if (auto res = fs->mount(cache_path.generic_string().c_str(), "mount://cache", false); res != 0) {
                throw std::runtime_error("[Engine] Cache mount failed.");
            }
            if (auto res = fs->mount(bin_path.generic_string().c_str(), "mount://bin", true); res != 0) {
                throw std::runtime_error("[Engine] Bin mount failed.");
            }
            if (auto res = fs->mount(app_path.generic_string().c_str(), "mount://app", true); res != 0) {
                throw std::runtime_error("[Engine] App mount failed.");
            }

            SANDBOX_INFO(ecs, "[Engine] VFS mounts ready (cache, bin, app).");
        }

        /// Scans app and bin mounts for modules and copies them into the writable cache.
        void build_local_modules_cache(flecs::world& ecs) {
            const auto& fs = ecs.get<sandbox::filesystem_service>().api;
            std::vector<std::filesystem::path> all_module_paths;

            sandbox_payload app_payload{};
            if (auto res = fs->list("mount://app/modules", false, &app_payload); res == 0 && app_payload.bytes) {
                auto fb_list = flatbuffers::GetRoot<sandbox::schemas::StringList>(app_payload.bytes);
                if (fb_list && fb_list->items()) {
                    for (const auto& item : *fb_list->items()) {
                        all_module_paths.push_back(std::filesystem::path(item->str()));
                    }
                }
                if (app_payload.free_func) app_payload.free_func(app_payload.bytes);
            } else {
                SANDBOX_WARN(ecs, "[Engine] No app modules found.");
            }

            sandbox_payload bin_payload{};
            if (auto res = fs->list("mount://bin/modules", false, &bin_payload); res == 0 && bin_payload.bytes) {
                auto fb_list = flatbuffers::GetRoot<sandbox::schemas::StringList>(bin_payload.bytes);
                if (fb_list && fb_list->items()) {
                    for (const auto& item : *fb_list->items()) {
                        all_module_paths.push_back(std::filesystem::path(item->str()));
                    }
                }
                if (bin_payload.free_func) bin_payload.free_func(bin_payload.bytes);
            } else {
                SANDBOX_WARN(ecs, "[Engine] No bin modules found.");
            }

            if (all_module_paths.empty()) {
                SANDBOX_INFO(ecs, "[Engine] No modules discovered.");
                return;
            }

            if (auto res = fs->mkdir("mount://cache/modules"); res != 0) {
                SANDBOX_ERROR(ecs, "[Engine] Cache mkdir failed.");
            }

            for (const auto& mod_path : all_module_paths) {
                if (mod_path.extension() == SANDBOX_COMPATIBLE_MODULE_EXTENSION) {
                    auto dest = std::filesystem::path("mount://cache/modules") / mod_path.filename();
                    if (auto res = fs->copy(mod_path.generic_string().c_str(), dest.generic_string().c_str()); res != 0) {
                        SANDBOX_WARN(ecs, "[Engine] Failed to cache '{}'", mod_path.filename().string());
                    }
                }
            }

            SANDBOX_INFO(ecs, "[Engine] Module cache built.");
        }

        /// Reads the application manifest and registers it as a global ECS entity.
        void parse_and_register_manifest(flecs::world& ecs) {
            const auto& fs = ecs.get<sandbox::filesystem_service>().api;

            sandbox_payload raw_data_res{};
            auto read_res = fs->read("mount://app/manifest.json", &raw_data_res);
            if (read_res != 0) {
                SANDBOX_WARN(ecs, "[Engine] manifest.json not found.");
                return;
            }

            if (raw_data_res.size == 0) {
                SANDBOX_WARN(ecs, "[Engine] manifest.json is empty.");
                if (raw_data_res.free_func) raw_data_res.free_func(raw_data_res.bytes);
                return;
            }

            std::string json_content(
                reinterpret_cast<const char*>(raw_data_res.bytes),
                raw_data_res.size);
            
            if (raw_data_res.free_func) raw_data_res.free_func(raw_data_res.bytes);

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
            sandbox_payload cached_res{};
            if (auto list_res = fs->list(cache_modules_dir.c_str(), false, &cached_res); list_res == 0 && cached_res.bytes) {
                auto fb_list = flatbuffers::GetRoot<sandbox::schemas::StringList>(cached_res.bytes);
                if (fb_list && fb_list->items()) {
                    for (const auto& item : *fb_list->items()) {
                        std::filesystem::path file(item->str());
                        if (file.extension() == SANDBOX_COMPATIBLE_MODULE_EXTENSION) {
                            auto vpath = std::filesystem::path(cache_modules_dir) / file.filename();
                            if (auto load_res = sandbox::internal::load(ecs, vpath.generic_string()); !load_res) {
                                SANDBOX_WARN(ecs, "[Engine] Failed to stage '{}': {}", file.filename().string(), load_res.error());
                            }
                        }
                    }
                }
                if (cached_res.free_func) cached_res.free_func(cached_res.bytes);
            } else {
                SANDBOX_WARN(ecs, "[Engine] Could not list cached modules.");
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
                        try {
                            boot.activate(module_name);
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
                    if (auto* runner = m_impl->ecs.get<sandbox::runner_service>().api; runner != nullptr) {
                        runner->quit();
                    }
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
                        if (auto* runner = m_impl->ecs.get<sandbox::runner_service>().api; runner != nullptr) {
                            runner->quit();
                        }
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
            if (auto* runner = m_impl->ecs.get<sandbox::runner_service>().api; runner != nullptr) {
                runner->run_sync(m_impl->ecs);
            }
        }
    }

} // namespace sandbox