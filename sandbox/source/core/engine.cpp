#include "sandbox/core/engine.h"
#include "sandbox/core/platform.h"
#include "sandbox/utilities/filesystem.h"
#include "sandbox/macros/logger.h"
#include <stdexcept>

#include "modules/logger.h"
#include "modules/runner.h"
#include "sandbox/core/plugin.h"

namespace sandbox {

    engine::engine() = default;

    engine::~engine() {
        finalize();
    }

    void engine::initialize(const properties& manifest) {
        ecs.reset();

        ecs.entity("::manifest").set<properties>(manifest);

        ecs.import<modules::logger>();
        ecs.import<modules::runner>();

        SANDBOX_INFO(ecs, "[Engine] Initializing...");

        std::string libraries_directory = manifest.get<std::string>({"path", "libraries"}).value_or("libraries");

        SANDBOX_DEBUG(ecs, "[Engine] Scanning plugin directory: '{}'", libraries_directory);

        try {
            load_libraries_from_directory(libraries_directory);
            SANDBOX_INFO(ecs, "[Engine] Initialization complete.");
        } catch (const std::exception& error) {
            SANDBOX_FATAL(ecs, "[Engine] Initialization failed: {}", error.what());
            throw;
        }
    }

    void engine::finalize() {
        ecs.reset();
    }

    void engine::load_libraries_from_directory(const std::filesystem::path& directory_path) {
        if (!filesystem::is_directory(directory_path)) {
            SANDBOX_ERROR_THROW(ecs, "Invalid plugin directory: {}", directory_path.string());
        }

        uint32_t loaded_plugins_counter = 0;

        for (const auto& library_path : filesystem::get_all_files_recursive(directory_path)) {
            if (library_path.extension() == SANDBOX_COMPATIBLE_MODULE_EXTENSION) {
                try {
                    SANDBOX_TRACE(ecs, "[Engine] Found module: '{}'", library_path.filename().string());
                    load_library(library_path);
                    loaded_plugins_counter++;
                } catch (const std::exception& error) {
                    SANDBOX_ERROR(ecs, "[Engine] Failed to load '{}': {}", library_path.filename().string(), error.what());
                }
            }
        }

        SANDBOX_INFO(ecs, "[Engine] Loaded {} plugins successfully.", loaded_plugins_counter);
    }

    void engine::load_library(const std::filesystem::path& library_path) {
        load_module_from_library(library_path, "SandboxLibraryMain");
    }

    void engine::load_module_from_library(const std::filesystem::path& library_path, const char* module_name) {
        if (!filesystem::is_file(library_path) || library_path.extension() != SANDBOX_COMPATIBLE_MODULE_EXTENSION) {
            SANDBOX_ERROR_THROW(ecs, "Invalid plugin file: {}", library_path.string());
        }

        std::string clean_filename = filesystem::strip_extension(library_path).string();

        SANDBOX_DEBUG(ecs, "[Engine] Linking symbol '{}::{}'", library_path.filename().string(), module_name);

        auto library = ecs_import_from_library(
            ecs.c_ptr(),
            clean_filename.c_str(),
            module_name
        );

        if (library) {
            SANDBOX_INFO(ecs, "[Engine] Mounted '{}'", library_path.filename().string());
        } else {
            SANDBOX_ERROR(ecs, "[Engine] Failed to mount '{}'", library_path.filename().string());
        }
    }

} // namespace sandbox