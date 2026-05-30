#pragma once

#include "ecs.h"
#include "../utilities/properties.h"
#include <filesystem>
#include <string>

namespace sandbox {

    class engine {
    public:
        engine();
        ~engine();

        engine(const engine&) = delete;
        engine& operator=(const engine&) = delete;

        engine(engine&&) noexcept = default;
        engine& operator=(engine&&) noexcept = default;

        void initialize(const properties& manifest);
        void finalize();

        void load_libraries_from_directory(const std::filesystem::path& directory_path);
        void load_library(const std::filesystem::path& library_path);
        void load_module_from_library(const std::filesystem::path& library_path, const std::string& module_name);

    public:
        world ecs;
    };

}