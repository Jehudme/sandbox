#include "sandbox/core/engine.h"
#include "sandbox/core/platform.h"
#include "sandbox/utilities/filesystem.h"
#include <stdexcept>

#include "modules/logger.h"
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

        load_libraries_from_directory(manifest.get<std::string>({"path", "libraries"}).value_or("libraries"));
    }

    void engine::finalize() {
        ecs.reset();
    }

    void engine::load_libraries_from_directory(const std::filesystem::path& directory_path) {
        if (!filesystem::is_directory(directory_path)) {
            throw std::runtime_error("Invalid module directory: " + directory_path.string());
        }

        for (const auto& library_path : filesystem::get_all_files_recursive(directory_path)) {
            if (library_path.extension() == SANDBOX_COMPATIBLE_MODULE_EXTENSION) {
                try {
                    load_library(library_path);
                } catch (...) {}
            }
        }
    }

    void engine::load_library(const std::filesystem::path& library_path) {
        load_module_from_library(library_path, "SandboxLibraryMain");
    }

    void engine::load_module_from_library(const std::filesystem::path& library_path, const char* module_name) {
        if (!filesystem::is_file(library_path) || library_path.extension() != SANDBOX_COMPATIBLE_MODULE_EXTENSION) {
            throw std::runtime_error("Invalid or unsupported library file: " + library_path.string());
        }

        ecs_import_from_library(
            ecs.c_ptr(),
            filesystem::strip_extension(library_path).string().c_str(),
            module_name
        );
    }

}
