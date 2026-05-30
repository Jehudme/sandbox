#pragma once

#include <filesystem>

#include "ecs.h"
#include "sandbox/utilities/properties.h"

namespace sandbox {

    class engine {
    public:
        engine() = default;
        ~engine();

        engine(const engine&) = delete;
        engine& operator=(const engine&) = delete;

        engine(engine&&) noexcept = default;
        engine& operator=(engine&&) noexcept = default;

        void initialize(const properties& manifest);
        void finalize();

        void load_libraries_from_directory(const std::filesystem::path& directory_path);
        void load_library(const std::filesystem::path& library_path);
        void load_module_from_library(const std::filesystem::path& library_path, const char* module_name);

    public:
        world ecs;
    };

}
