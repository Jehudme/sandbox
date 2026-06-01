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
    engine::engine() = default;

    engine::~engine() {
        finalize();
    }

    // ============================================================================
    // Core Lifecycle
    // ============================================================================

    void engine::initialize(const std::filesystem::path& root_mount_path) {
        sandbox::configure_plugin_os_api();


        import_core_modules();

        // 1. Get the absolute paths for both execution roots
        std::filesystem::path launcher_physical_path = std::filesystem::current_path();
        std::filesystem::path core_physical_path = std::filesystem::absolute(root_mount_path);

        SANDBOX_INFO(ecs, "[Engine] Mounting launcher base layout from: '{}'", launcher_physical_path.string());
        SANDBOX_INFO(ecs, "[Engine] Mounting application core layout from: '{}'", core_physical_path.string());

        // 2. Mount both layout paths securely
        SANDBOX_VFS_MOUNT(ecs, launcher_physical_path, "mount://launcher", true);
        SANDBOX_VFS_MOUNT(ecs, core_physical_path, "mount://core", true);

        // 3. Instantly execute the synchronous read
        //std::vector<std::byte> manifest_data = SANDBOX_VFS_EXEC_READ(ecs, "mount://core/manifest.json");

        process_manifest_payload(std::move(std::vector<std::byte>()));

        load_libraries_from_directory("mount://launcher/libraries");
    }

    void engine::finalize() {
        ecs.reset();
    }

    // ============================================================================
    // Initialization Helpers
    // ============================================================================

    void engine::import_core_modules() {
        ecs.import<modules::logger>();
        ecs.import<modules::runner>();
        ecs.import<modules::filesystem>();
    }

    void engine::process_manifest_payload(std::vector<std::byte>&& data) {
        //if (data.empty()) {
        //    SANDBOX_FATAL(ecs, "[Engine] Failed to read manifest.json from mounted core package.");
        //    SANDBOX_RUNNER_QUIT(ecs);
        //    return;
        //}

        // 1. Instantly parse the raw byte payload
        properties manifest(data);

        // 2. Inject configuration into ECS
        ecs.entity("::manifest").set<properties>(manifest);
        SANDBOX_INFO(ecs, "[Engine] Manifest successfully parsed and registered.");

        // 3. Fall back gracefully to a virtual layout directory path if omitted from properties
        std::string libraries_directory = manifest.get<std::string>({"path", "libraries"}).value_or("mount://core/libraries");
        SANDBOX_DEBUG(ecs, "[Engine] Scanning virtual plugin directory: '{}'", libraries_directory);

        try {
            // 4. Safely execute routing scans sequentially with absolute path verification
            load_libraries_from_directory(libraries_directory);
            SANDBOX_INFO(ecs, "[Engine] Initialization sequence complete.");
        } catch (const std::exception& error) {
            SANDBOX_FATAL(ecs, "[Engine] Initialization failed during plugin loading: {}", error.what());
            SANDBOX_RUNNER_QUIT(ecs);
        }
    }

    // ============================================================================
    // Plugin Loading Subsystem (VFS Routed)
    // ============================================================================

    void engine::load_libraries_from_directory(const std::filesystem::path& virtual_directory_path) {
        uint32_t loaded_plugins_counter = 0;

        // 1. Fetch a flat vector of complete virtual paths recursively
        std::vector<std::filesystem::path> absolute_virtual_paths = SANDBOX_VFS_EXEC_LIST(ecs, virtual_directory_path, true);

        for (const auto& path : absolute_virtual_paths) {
            auto meta = SANDBOX_VFS_EXEC_STATE(ecs, path);

            if (meta.type != events::vfs::file_type::regular && meta.type != events::vfs::file_type::symlink) continue;
            if (path.extension() != SANDBOX_COMPATIBLE_MODULE_EXTENSION) continue;

            try {
                SANDBOX_TRACE(ecs, "[Engine] VFS routed physical module: '{}'", path.filename().string());
                load_library(SANDBOX_VFS_EXEC_ABSOLUTE(ecs, path));
                loaded_plugins_counter++;
            } catch (const std::exception& error) {
                SANDBOX_ERROR(ecs, "[Engine] Failed to load '{}': {}", path.filename().string(), error.what());
            }
        }

        SANDBOX_INFO(ecs, "[Engine] Loaded {} plugins successfully through recursive VFS mapping.", loaded_plugins_counter);
    }

    void engine::load_library(const std::filesystem::path& library_path) {
        // Force the entry point name into a static string with an infinite lifetime
        static const std::string entry_point = "SandboxLibraryMain";
        load_module_from_library(library_path, entry_point.c_str());
    }

    void engine::load_module_from_library(const std::filesystem::path& physical_library_path, const char* module_name) {
        if (!std::filesystem::is_regular_file(physical_library_path)) {
            SANDBOX_ERROR_THROW(ecs, "Invalid physical plugin file: {}", physical_library_path.string());
        }

        // Keep the clean filename alive as a local variable during this scope
        std::string clean_filename = filesystem::strip_extension(physical_library_path).string();

        SANDBOX_DEBUG(ecs, "[Engine] Linking physical symbol '{}::{}'", physical_library_path.filename().string(), module_name);

        // Hand the physical disk path and the permanently alive string securely to Flecs
        auto library = ecs_import_from_library(
            ecs.c_ptr(),
            clean_filename.c_str(),
            module_name
        );

        if (library) {
            SANDBOX_INFO(ecs, "[Engine] Mounted '{}' successfully.", physical_library_path.filename().string());
        } else {
            SANDBOX_ERROR(ecs, "[Engine] Failed to mount '{}'.", physical_library_path.filename().string());
        }
    }
} // namespace sandbox