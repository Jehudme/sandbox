#include "sandbox/core/engine.h"
#include "sandbox/core/exports.h"
#include "sandbox/utilities/filesystem.h"
#include <stdexcept>

namespace sandbox {

    engine::engine() {
    }

    engine::~engine() {
        finalize();
    }

    void engine::initialize(const properties& manifest) {
        ecs.reset();

        std::string modules_path = manifest
            .get<std::string>({"path", "libraries"})
            .value_or("libraries");

        load_libraries_from_directory(modules_path);
    }

    void engine::finalize() {
        ecs.reset();
    }

    void engine::load_libraries_from_directory(const std::filesystem::path& directory_path) {
        if (!filesystem::is_directory(directory_path)) {
            throw std::runtime_error("Cannot load modules: Provided path is not a valid directory -> " + directory_path.string());
        }

        auto all_files = filesystem::get_all_files_recursive(directory_path);

        for (const auto& library_path : all_files) {
            if (library_path.extension() == SANDBOX_COMPATIBLE_MODULE_EXTENSION) {
                try {
                    load_library(library_path);
                }
                catch (const std::runtime_error&) {
                }
            }
        }
    }

    void engine::load_library(const std::filesystem::path& library_path) {
        std::string inferred_module_name = library_path.stem().string();
        load_module_from_library(library_path, inferred_module_name);
    }

    void engine::load_module_from_library(const std::filesystem::path& library_path, const std::string& module_name) {
        if (!filesystem::exists(library_path)) {
            throw std::runtime_error("Library file does not exist on disk: " + library_path.string());
        }

        if (library_path.extension() != SANDBOX_COMPATIBLE_MODULE_EXTENSION) {
            throw std::runtime_error("Unsupported binary file extension for library: " + library_path.string());
        }

        std::filesystem::path clean_library_path = filesystem::strip_extension(library_path);

        ecs_import_from_library(
            ecs.c_ptr(),
            clean_library_path.string().c_str(),
            module_name.c_str()
        );
    }

} // namespace sandbox